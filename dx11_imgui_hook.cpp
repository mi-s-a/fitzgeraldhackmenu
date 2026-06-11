#include "pch.h"

#include "dx11_imgui_hook.h"

#include "debug_console.h"
#include "imgui_overlay.h"

#include <dxgi.h>
#include <kiero.h>

#include <atomic>
#include <thread>

namespace
{
    using PresentFn = HRESULT(__stdcall*)(IDXGISwapChain* swapChain, UINT syncInterval, UINT flags);

    std::atomic_bool g_started = false;
    std::atomic_bool g_stopRequested = false;

    PresentFn g_originalPresent = nullptr;
    HWND g_window = nullptr;
    WNDPROC g_originalWndProc = nullptr;
    ID3D11Device* g_device = nullptr;
    ID3D11DeviceContext* g_deviceContext = nullptr;
    ID3D11RenderTargetView* g_renderTargetView = nullptr;
    bool g_loggedFirstPresent = false;
    bool g_loggedFirstRender = false;
    bool g_cursorUnlocked = false;
    bool g_lastMenuVisible = false;

    const char* KieroStatusToString(kiero::Status::Enum status)
    {
        switch (status)
        {
        case kiero::Status::Success:
            return "Success";
        case kiero::Status::UnknownError:
            return "UnknownError";
        case kiero::Status::NotSupportedError:
            return "NotSupportedError";
        case kiero::Status::ModuleNotFoundError:
            return "ModuleNotFoundError";
        case kiero::Status::AlreadyInitializedError:
            return "AlreadyInitializedError";
        case kiero::Status::NotInitializedError:
            return "NotInitializedError";
        default:
            return "Unrecognized";
        }
    }

    void ReleaseRenderTarget()
    {
        if (g_renderTargetView != nullptr)
        {
            g_renderTargetView->Release();
            g_renderTargetView = nullptr;
        }
    }

    bool CreateRenderTarget(IDXGISwapChain* swapChain)
    {
        ReleaseRenderTarget();

        ID3D11Texture2D* backBuffer = nullptr;
        if (FAILED(swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer))) || backBuffer == nullptr)
        {
            fitzgeraldhackmenu::Log("failed to get swap-chain back buffer");
            return false;
        }

        const HRESULT result = g_device->CreateRenderTargetView(backBuffer, nullptr, &g_renderTargetView);
        backBuffer->Release();
        fitzgeraldhackmenu::Log("CreateRenderTargetView result=0x%08X", static_cast<unsigned int>(result));
        return SUCCEEDED(result);
    }

    void SetCursorVisible(bool visible)
    {
        if (visible)
        {
            while (ShowCursor(TRUE) < 0)
            {
            }
            return;
        }

        while (ShowCursor(FALSE) >= 0)
        {
        }
    }

    RECT GetClientScreenRect(HWND window)
    {
        RECT rect = {};
        GetClientRect(window, &rect);

        POINT topLeft = { rect.left, rect.top };
        POINT bottomRight = { rect.right, rect.bottom };
        ClientToScreen(window, &topLeft);
        ClientToScreen(window, &bottomRight);

        rect.left = topLeft.x;
        rect.top = topLeft.y;
        rect.right = bottomRight.x;
        rect.bottom = bottomRight.y;
        return rect;
    }

    void ApplyCursorState(bool menuVisible)
    {
        if (g_window == nullptr || g_cursorUnlocked == menuVisible)
        {
            return;
        }

        if (menuVisible)
        {
            ReleaseCapture();
            ClipCursor(nullptr);
            SetCursorVisible(true);
            g_cursorUnlocked = true;
            fitzgeraldhackmenu::Log("cursor unlocked for menu");
            return;
        }

        const RECT clipRect = GetClientScreenRect(g_window);
        ClipCursor(&clipRect);
        SetCursorVisible(false);
        g_cursorUnlocked = false;
        fitzgeraldhackmenu::Log("cursor locked to game window");
    }

    LRESULT CALLBACK HookedWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        if (message == WM_KEYUP && wParam == VK_INSERT)
        {
            fitzgeraldhackmenu::SetImGuiOverlayVisible(!fitzgeraldhackmenu::IsImGuiOverlayVisible());
            ApplyCursorState(fitzgeraldhackmenu::IsImGuiOverlayVisible());
            fitzgeraldhackmenu::Log("Insert pressed, overlay visible=%s", fitzgeraldhackmenu::IsImGuiOverlayVisible() ? "true" : "false");
            return 0;
        }

        if (fitzgeraldhackmenu::IsImGuiOverlayVisible() &&
            fitzgeraldhackmenu::HandleImGuiOverlayMessage(hwnd, message, wParam, lParam))
        {
            return 1;
        }

        return CallWindowProc(g_originalWndProc, hwnd, message, wParam, lParam);
    }

    bool InitializeFromSwapChain(IDXGISwapChain* swapChain)
    {
        if (g_device != nullptr)
        {
            return true;
        }

        if (FAILED(swapChain->GetDevice(__uuidof(ID3D11Device), reinterpret_cast<void**>(&g_device))) || g_device == nullptr)
        {
            fitzgeraldhackmenu::Log("failed to get ID3D11Device from swap chain");
            return false;
        }

        g_device->GetImmediateContext(&g_deviceContext);
        fitzgeraldhackmenu::Log("got D3D11 device/context");

        DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
        if (FAILED(swapChain->GetDesc(&swapChainDesc)) || swapChainDesc.OutputWindow == nullptr)
        {
            fitzgeraldhackmenu::Log("failed to get swap-chain window");
            return false;
        }

        g_window = swapChainDesc.OutputWindow;
        fitzgeraldhackmenu::Log("swap-chain window=0x%p", g_window);
        if (!CreateRenderTarget(swapChain))
        {
            return false;
        }

        g_originalWndProc = reinterpret_cast<WNDPROC>(
            SetWindowLongPtr(g_window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(HookedWndProc)));
        fitzgeraldhackmenu::Log("WndProc hooked, original=0x%p", g_originalWndProc);

        const bool initialized = fitzgeraldhackmenu::InitializeImGuiOverlay(g_window, g_device, g_deviceContext);
        fitzgeraldhackmenu::Log("InitializeImGuiOverlay=%s", initialized ? "true" : "false");
        g_lastMenuVisible = fitzgeraldhackmenu::IsImGuiOverlayVisible();
        ApplyCursorState(g_lastMenuVisible);
        return initialized;
    }

    HRESULT __stdcall HookedPresent(IDXGISwapChain* swapChain, UINT syncInterval, UINT flags)
    {
        if (!g_loggedFirstPresent)
        {
            fitzgeraldhackmenu::Log("first Present hook call");
            g_loggedFirstPresent = true;
        }

        if (InitializeFromSwapChain(swapChain))
        {
            const bool menuVisible = fitzgeraldhackmenu::IsImGuiOverlayVisible();
            if (menuVisible != g_lastMenuVisible)
            {
                ApplyCursorState(menuVisible);
                g_lastMenuVisible = menuVisible;
            }

            if (g_renderTargetView == nullptr)
            {
                CreateRenderTarget(swapChain);
            }

            if (g_renderTargetView != nullptr)
            {
                g_deviceContext->OMSetRenderTargets(1, &g_renderTargetView, nullptr);
                fitzgeraldhackmenu::RenderImGuiOverlay();
                if (!g_loggedFirstRender)
                {
                    fitzgeraldhackmenu::Log("first ImGui render submitted");
                    g_loggedFirstRender = true;
                }
            }
        }

        return g_originalPresent(swapChain, syncInterval, flags);
    }

    void HookThread()
    {
        fitzgeraldhackmenu::Log("DX11 hook thread started");
        int attempts = 0;

        while (!g_stopRequested.load())
        {
            const kiero::Status::Enum status = kiero::init(kiero::RenderType::D3D11);
            ++attempts;

            if (attempts == 1 || attempts % 20 == 0)
            {
                fitzgeraldhackmenu::Log(
                    "kiero init attempt %d status=%s d3d11=%s dxgi=%s",
                    attempts,
                    KieroStatusToString(status),
                    GetModuleHandleW(L"d3d11.dll") != nullptr ? "loaded" : "not loaded",
                    GetModuleHandleW(L"dxgi.dll") != nullptr ? "loaded" : "not loaded");
            }

            if (status == kiero::Status::Success || status == kiero::Status::AlreadyInitializedError)
            {
                const kiero::Status::Enum bindStatus = kiero::bind(8, reinterpret_cast<void**>(&g_originalPresent), reinterpret_cast<void*>(HookedPresent));
                fitzgeraldhackmenu::Log("kiero bind Present status=%s original=0x%p", KieroStatusToString(bindStatus), g_originalPresent);
                if (bindStatus == kiero::Status::Success)
                {
                    fitzgeraldhackmenu::Log("DX11 Present hook installed");
                    return;
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(250));
        }
    }
}

namespace fitzgeraldhackmenu
{
    void StartDx11ImGuiHook()
    {
        bool expected = false;
        if (!g_started.compare_exchange_strong(expected, true))
        {
            Log("DX11 hook already started");
            return;
        }

        g_stopRequested = false;
        Log("starting DX11 ImGui hook");
        std::thread(HookThread).detach();
    }

    void StopDx11ImGuiHook()
    {
        if (!g_started.exchange(false))
        {
            Log("DX11 hook stop requested, but it was not started");
            return;
        }

        Log("stopping DX11 ImGui hook");
        g_stopRequested = true;

        if (g_cursorUnlocked)
        {
            ApplyCursorState(false);
        }

        ShutdownImGuiOverlay();

        if (g_window != nullptr && g_originalWndProc != nullptr)
        {
            SetWindowLongPtr(g_window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(g_originalWndProc));
            g_originalWndProc = nullptr;
        }

        ReleaseRenderTarget();

        if (g_deviceContext != nullptr)
        {
            g_deviceContext->Release();
            g_deviceContext = nullptr;
        }

        if (g_device != nullptr)
        {
            g_device->Release();
            g_device = nullptr;
        }

        kiero::shutdown();
    }
}
