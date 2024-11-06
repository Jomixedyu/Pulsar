#include "Windows/StatusBarWindow.h"
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include "EditorLogRecorder.h"

namespace pulsared
{
    static ImVec2 Add(ImVec2 a, ImVec2 b)
    {
        return ImVec2(a.x + b.x, a.y + b.y);
    }

    bool BeginMainStatusBar()
    {
        ImGuiContext& g = *GImGui;
        ImGuiViewportP* viewport = (ImGuiViewportP*)(void*)ImGui::GetMainViewport();

        // Notify of viewport change so GetFrameHeight() can be accurate in case of DPI change
        ImGui::SetCurrentViewport(NULL, viewport);

        // For the main menu bar, which cannot be moved, we honor g.Style.DisplaySafeAreaPadding to ensure text can be visible on a TV set.
        // FIXME: This could be generalized as an opt-in way to clamp window->DC.CursorStartPos to avoid SafeArea?
        // FIXME: Consider removing support for safe area down the line... it's messy. Nowadays consoles have support for TV calibration in OS settings.
        g.NextWindowData.MenuBarOffsetMinVal = ImVec2(g.Style.DisplaySafeAreaPadding.x, ImMax(g.Style.DisplaySafeAreaPadding.y - g.Style.FramePadding.y, 0.0f));
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar;
        float height = ImGui::GetFrameHeight();
        bool is_open = ImGui::BeginViewportSideBar("##MainStatusBar", viewport, ImGuiDir_Down, height, window_flags);
        g.NextWindowData.MenuBarOffsetMinVal = ImVec2(0.0f, 0.0f);

        if (is_open)
            ImGui::BeginMenuBar();
        else
            ImGui::End();
        return is_open;
    }



    void StatusBarWindow::OnOpen()
    {
       
    }

    void StatusBarWindow::OnClose()
    {

    }

    void StatusBarWindow::OnDrawImGui(float dt)
    {
        ImGui::PushStyleColor(ImGuiCol_MenuBarBg, { 0.08f, 0.08f, 0.08f, 1.f });
        if (BeginMainStatusBar())
        {
            if (EditorLogRecorder::loglist.size() != 0)
            {
                ImGui::Text(EditorLogRecorder::loglist[EditorLogRecorder::loglist.size() - 1].record_info.c_str());
            }
            ImGui::EndMainMenuBar();
        }
        ImGui::PopStyleColor();
    }

    void StatusBarWindow::DrawImGui(float dt)
    {
        OnDrawImGui(dt);
    }

    void StatusBarWindow::OnWindowResize()
    {

    }

}