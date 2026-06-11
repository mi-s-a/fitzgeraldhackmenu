#pragma once

#include <d3d11.h>
#include <Windows.h>

namespace fitzgeraldhackmenu
{
    bool InitializeImGuiOverlay(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* deviceContext);
    void ShutdownImGuiOverlay();
    void RenderImGuiOverlay();
    bool HandleImGuiOverlayMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void SetImGuiOverlayVisible(bool visible);
    bool IsImGuiOverlayVisible();
}
