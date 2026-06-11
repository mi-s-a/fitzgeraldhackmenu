#include "pch.h"

#include "imgui_overlay.h"

#include "il2cpp_resolver_integration.h"

#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
#include "auth.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

namespace
{
    bool g_initialized = false;
    bool g_visible = true;
    std::string accountid;
    enum class MenuTab {
        Main,
        Visuals,
        Misc,
        Settings
    };
    struct MenuSettings {
        bool enabled = true;
        bool placeholderb = false;
        int placeholderi = 1;
        int menuKey = VK_INSERT;
        float placeholderf = 1.0f;
        char accountIdInput[64] = "1";
    };
    MenuTab g_activeTab = MenuTab::Main;
    MenuSettings g_settings;
    const char* ToDisplayString(fitzgeraldhackmenu::Il2CppResolverState state) {
        switch (state)
        {
        case fitzgeraldhackmenu::Il2CppResolverState::NotStarted:
            return "not started";
        case fitzgeraldhackmenu::Il2CppResolverState::WaitingForGameAssembly:
            return "waiting for GameAssembly.dll";
        case fitzgeraldhackmenu::Il2CppResolverState::Initialized:
            return "initialized";
        case fitzgeraldhackmenu::Il2CppResolverState::Failed:
            return "failed";
        default:
            return "unknown";
        }
    }

    void SetupStyle()
    {
        ImGuiStyle& style = ImGui::GetStyle();

        // Layout
        style.WindowPadding = ImVec2(14.0f, 12.0f);
        style.FramePadding = ImVec2(9.0f, 5.0f);
        style.ItemSpacing = ImVec2(10.0f, 8.0f);
        style.ItemInnerSpacing = ImVec2(8.0f, 6.0f);
        style.ScrollbarSize = 12.0f;

        // Rounding
        style.WindowRounding = 8.0f;
        style.ChildRounding = 8.0f;
        style.FrameRounding = 6.0f;
        style.PopupRounding = 8.0f;
        style.ScrollbarRounding = 8.0f;
        style.GrabRounding = 6.0f;

        // Borders
        style.WindowBorderSize = 1.0f;
        style.ChildBorderSize = 1.0f;
        style.FrameBorderSize = 1.0f;

        ImVec4* colors = style.Colors;

        // Text
        colors[ImGuiCol_Text] = ImVec4(0.95f, 0.95f, 0.96f, 1.00f);
        colors[ImGuiCol_TextDisabled] = ImVec4(0.55f, 0.55f, 0.58f, 1.00f);

        // Backgrounds
        colors[ImGuiCol_WindowBg] = ImVec4(0.09f, 0.09f, 0.10f, 1.00f);
        colors[ImGuiCol_ChildBg] = ImVec4(0.11f, 0.11f, 0.12f, 1.00f);
        colors[ImGuiCol_PopupBg] = ImVec4(0.12f, 0.12f, 0.13f, 1.00f);

        // Borders
        colors[ImGuiCol_Border] = ImVec4(0.25f, 0.25f, 0.27f, 1.00f);
        colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

        // Frames / Inputs
        colors[ImGuiCol_FrameBg] = ImVec4(0.14f, 0.14f, 0.15f, 1.00f);
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.18f, 0.18f, 0.20f, 1.00f);
        colors[ImGuiCol_FrameBgActive] = ImVec4(0.24f, 0.24f, 0.26f, 1.00f);

        // Title Bars
        colors[ImGuiCol_TitleBg] = ImVec4(0.07f, 0.07f, 0.08f, 1.00f);
        colors[ImGuiCol_TitleBgActive] = ImVec4(0.10f, 0.10f, 0.11f, 1.00f);
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.07f, 0.07f, 0.08f, 1.00f);

        // Menu Bar
        colors[ImGuiCol_MenuBarBg] = ImVec4(0.11f, 0.11f, 0.12f, 1.00f);

        // Scrollbars
        colors[ImGuiCol_ScrollbarBg] = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
        colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.45f, 0.45f, 0.48f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.65f, 0.65f, 0.68f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.90f, 0.90f, 0.92f, 1.00f);

        // Checkboxes
        colors[ImGuiCol_CheckMark] = ImVec4(0.95f, 0.95f, 0.95f, 1.00f);

        // Sliders
        colors[ImGuiCol_SliderGrab] = ImVec4(0.75f, 0.75f, 0.78f, 1.00f);
        colors[ImGuiCol_SliderGrabActive] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);

        // Buttons
        colors[ImGuiCol_Button] = ImVec4(0.16f, 0.16f, 0.17f, 1.00f);
        colors[ImGuiCol_ButtonHovered] = ImVec4(0.22f, 0.22f, 0.24f, 1.00f);
        colors[ImGuiCol_ButtonActive] = ImVec4(0.32f, 0.32f, 0.34f, 1.00f);

        // Headers / TreeNodes / Selectables
        colors[ImGuiCol_Header] = ImVec4(0.18f, 0.18f, 0.20f, 1.00f);
        colors[ImGuiCol_HeaderHovered] = ImVec4(0.28f, 0.28f, 0.30f, 1.00f);
        colors[ImGuiCol_HeaderActive] = ImVec4(0.40f, 0.40f, 0.42f, 1.00f);

        // Separators
        colors[ImGuiCol_Separator] = ImVec4(0.25f, 0.25f, 0.27f, 1.00f);
        colors[ImGuiCol_SeparatorHovered] = ImVec4(0.65f, 0.65f, 0.68f, 1.00f);
        colors[ImGuiCol_SeparatorActive] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);

        // Resize Grip
        colors[ImGuiCol_ResizeGrip] = ImVec4(0.75f, 0.75f, 0.78f, 0.20f);
        colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.90f, 0.90f, 0.92f, 0.50f);
        colors[ImGuiCol_ResizeGripActive] = ImVec4(1.00f, 1.00f, 1.00f, 0.90f);

        // Plots
        colors[ImGuiCol_PlotLines] = ImVec4(0.85f, 0.85f, 0.88f, 1.00f);
        colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);

        colors[ImGuiCol_PlotHistogram] = ImVec4(0.85f, 0.85f, 0.88f, 1.00f);
        colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);

        // Selection
        colors[ImGuiCol_TextSelectedBg] = ImVec4(0.85f, 0.85f, 0.90f, 0.25f);

        // Misc
        colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);

        colors[ImGuiCol_NavHighlight] = ImVec4(0.85f, 0.85f, 0.90f, 0.60f);
        colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
        colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.20f);
        colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.60f);
    }

    bool NavButton(const char* label, MenuTab tab) {
        const bool selected = g_activeTab == tab;
        if (selected) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.16f, 0.16f, 0.17f, 1.00f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.22f, 0.22f, 0.24f, 1.00f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.32f, 0.32f, 0.34f, 1.00f));
        }
        const bool clicked = ImGui::Button(label, ImVec2(-1.0f, 34.0f));
        if (selected) {
            ImGui::PopStyleColor(3);

            const ImVec2 min = ImGui::GetItemRectMin();
            const ImVec2 max = ImGui::GetItemRectMax();
            ImGui::GetWindowDrawList()->AddRectFilled(
                ImVec2(min.x, min.y + 5.0f),
                ImVec2(min.x + 3.0f, max.y - 5.0f),
                ImGui::GetColorU32(ImVec4(0.33f, 0.33f, 0.35f, 1.00f)),
                1.5f);
        }
        if (clicked) {
            g_activeTab = tab;
        }
        return clicked;
    }

    bool BeginSection(const char* title, const ImVec2& size) {
        ImGui::BeginChild(title, size, true);
        ImGui::TextUnformatted(title);
        ImGui::Separator();
        ImGui::Spacing();
        return true;
    }

    void EndSection() { ImGui::EndChild(); }

    void KeybindPlaceholder(const char* label, int key) {
        ImGui::TextUnformatted(label);
        ImGui::SameLine(180.0f);
        ImGui::Button(key == VK_INSERT ? "Insert" : "Unbound", ImVec2(100.0f, 0.0f));
    }

    void renderplaceholder1() {
        const float halfWidth = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * 0.5f;
        BeginSection("main", ImVec2(halfWidth, 0.0f));
        ImGui::Checkbox("menu", &g_settings.enabled);
        ImGui::Checkbox("placeholder", &g_settings.placeholderb);
        ImGui::Spacing();
        ImGui::TextUnformatted("id:");
        if (ImGui::InputText("##acc_id", g_settings.accountIdInput, IM_ARRAYSIZE(g_settings.accountIdInput))){
            fitzgeraldhackmenu::accountid = g_settings.accountIdInput;
        }
        ImGui::Spacing();
        if (ImGui::Button("resolveing", ImVec2(-1.0f, 0.0f))) {

        }
        EndSection();
    }

    void renderplaceholder2(){
        const float halfWidth = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * 0.5f;
        BeginSection("visuals", ImVec2(halfWidth, 0.0f));
        ImGui::SliderFloat("placeholder", &g_settings.placeholderf, 20.0f, 500.0f, "%.0f px");
        EndSection();
    }

    void renderplaceholder3() {
        BeginSection("misc", ImVec2(0.0f, 0.0f));
        KeybindPlaceholder("key", g_settings.menuKey);
        EndSection();
    }

    void RenderMenu() {
        ImGui::SetNextWindowSize(ImVec2(850.0f, 511.0f), ImGuiCond_FirstUseEver);
        const ImGuiWindowFlags flags =
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoScrollWithMouse;

        if (!ImGui::Begin("fitzgerald.lol", &g_visible, flags))
        {
            ImGui::End();
            return;
        }
        ImGui::BeginChild("Navigation", ImVec2(128.0f, 0.0f), true);
        ImGui::TextUnformatted("tabs");
        ImGui::Separator();
        ImGui::Spacing();
        NavButton("main", MenuTab::Main);
        NavButton("visuals", MenuTab::Visuals);
        NavButton("misc", MenuTab::Misc);
        ImGui::EndChild();
        ImGui::SameLine();
        ImGui::BeginChild("Content", ImVec2(0.0f, 0.0f), false);
        switch (g_activeTab)
        {
        case MenuTab::Main:
            renderplaceholder1();
            break;
        case MenuTab::Visuals:
            renderplaceholder2();
            break;
        case MenuTab::Misc:
            renderplaceholder3();
            break;
        }
        ImGui::EndChild();
        ImGui::End();
    }
}

namespace fitzgeraldhackmenu {
    bool InitializeImGuiOverlay(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* deviceContext) {
        if (g_initialized) {
            return true;
        }

        if (hwnd == nullptr || device == nullptr || deviceContext == nullptr) {
            return false;
        }
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\consola.ttf", 13.0f);
        SetupStyle();
        if (!ImGui_ImplWin32_Init(hwnd)) {
            ImGui::DestroyContext();
            return false;
        }

        if (!ImGui_ImplDX11_Init(device, deviceContext)) {
            ImGui_ImplWin32_Shutdown();
            ImGui::DestroyContext();
            return false;
        }
        g_initialized = true;
        return true;
    }

    void ShutdownImGuiOverlay() {
        if (!g_initialized) {
            return;
        }
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
        g_initialized = false;
    }

    void RenderImGuiOverlay() {
        if (!g_initialized || !g_visible) {
            return;
        }
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        RenderMenu();
        ImGui::Render();
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    }

    bool HandleImGuiOverlayMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
        if (!g_initialized) {
            return false;
        }
        return ImGui_ImplWin32_WndProcHandler(hwnd, message, wParam, lParam) != 0;
    }

    void SetImGuiOverlayVisible(bool visible)  {
        g_visible = visible;
    }

    bool IsImGuiOverlayVisible() {
        return g_visible;
    }
}
