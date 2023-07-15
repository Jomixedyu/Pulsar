#include "Windows/StatusBarWindow.h"
#include <ThirdParty/imgui/imgui.h>
#include <ThirdParty/imgui/imgui_internal.h>

namespace pulsared
{
    static ImVec2 Add(ImVec2 a, ImVec2 b)
    {
        return ImVec2(a.x + b.x, a.y + b.y);
    }
    bool HY_ImGui_BeginMainStatusBar()
    {
        ImGuiContext& g = *GImGui;
        ImGuiViewportP* viewport = g.Viewports[0];
        ImGuiWindow* menu_bar_window = ImGui::FindWindowByName("##MainStatusBar");

        // For the main menu bar, which cannot be moved, we honor g.Style.DisplaySafeAreaPadding to ensure text can be visible on a TV set.
        g.NextWindowData.MenuBarOffsetMinVal = ImVec2(g.Style.DisplaySafeAreaPadding.x, ImMax(g.Style.DisplaySafeAreaPadding.y - g.Style.FramePadding.y, 0.0f));

        // Get our rectangle at the top of the work area
        //__debugbreak();
        if (menu_bar_window == NULL || menu_bar_window->BeginCount == 0)
        {
            // Set window position
            // We don't attempt to calculate our height ahead, as it depends on the per-viewport font size. However menu-bar will affect the minimum window size so we'll get the right height.
            ImVec2 menu_bar_pos = Add(viewport->Pos, viewport->WorkOffsetMin);
            ImVec2 menu_bar_size = ImVec2(viewport->Size.x - viewport->WorkOffsetMin.x + viewport->WorkOffsetMin.x, 1.0f);
            ImGui::SetNextWindowPos(menu_bar_pos);
            ImGui::SetNextWindowSize(menu_bar_size);
        }

        // Create window
        ImGui::SetNextWindowViewport(viewport->ID); // Enforce viewport so we don't create our own viewport when ImGuiConfigFlags_ViewportsNoMerge is set.
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(0, 0));    // Lift normal size constraint, however the presence of a menu-bar will give us the minimum height we want.
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar;
        bool is_open = ImGui::Begin("##MainStatusBar", NULL, window_flags) && ImGui::BeginMenuBar();
        ImGui::PopStyleVar(2);

        // Report our size into work area (for next frame) using actual window size
        menu_bar_window = ImGui::GetCurrentWindow();
        if (menu_bar_window->BeginCount == 1)
            viewport->WorkOffsetMin.y += menu_bar_window->Size.y;

        g.NextWindowData.MenuBarOffsetMinVal = ImVec2(0.0f, 0.0f);
        if (!is_open)
        {
            ImGui::End();
            return false;
        }
        return true; //-V1020
    }

    void HY_ImGui_EndMainStatusBar()
    {
        ImGui::EndMenuBar();

        // When the user has left the menu layer (typically: closed menus through activation of an item), we restore focus to the previous window
        // FIXME: With this strategy we won't be able to restore a NULL focus.
        //ImGuiContext& g = *GImGui;
        //if (g.CurrentWindow == g.NavWindow && g.NavLayer == ImGuiNavLayer_Main && !g.NavAnyRequest)
        //    ImGui::FocusTopMostWindowUnderOne(g.NavWindow, NULL);

        //ImGui::End();
    }


    void StatusBarWindow::OnOpen()
    {
       
    }

    void StatusBarWindow::OnClose()
    {

    }

    void StatusBarWindow::OnDrawImGui()
    {
        //if (HY_ImGui_BeginMainStatusBar())
        //{
        //    HY_ImGui_EndMainStatusBar();
        //}
    }

    void StatusBarWindow::OnWindowResize()
    {

    }

}