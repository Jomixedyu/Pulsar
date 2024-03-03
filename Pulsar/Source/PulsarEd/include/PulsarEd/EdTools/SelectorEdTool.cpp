#include "SelectorEdTool.h"
#include <imgui/imgui.h>

namespace pulsared
{

    void SelectorEdTool::Begin()
    {
        base::Begin();
        m_frameSelector = false;
        Logger::Log("SelectorTool");
    }
    static bool IsModifilerKeysDown()
    {
        return
        ImGui::IsKeyDown(ImGuiKey_LeftAlt)
        || ImGui::IsKeyDown(ImGuiKey_RightAlt)
        || ImGui::IsKeyDown(ImGuiKey_LeftShift)
        || ImGui::IsKeyDown(ImGuiKey_RightShift)
        || ImGui::IsKeyDown(ImGuiKey_LeftCtrl)
        || ImGui::IsKeyDown(ImGuiKey_RightCtrl);
    }

    void SelectorEdTool::Tick(float dt)
    {
        base::Tick(dt);
        if (m_enableSelect && ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !IsModifilerKeysDown())
        {
            m_frameSelector = true;
            auto startpos = ImGui::GetMousePos();
            m_frameSelectorStartPos = { startpos.x, startpos.y };
        }
        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
        {
            m_frameSelector = false;
        }

        if (m_frameSelector)
        {
            ImDrawList* drawList = ImGui::GetWindowDrawList();
            auto curpos = ImGui::GetMousePos();
            auto startpos = ImVec2{m_frameSelectorStartPos.x, m_frameSelectorStartPos.y};
            auto fillColor = IM_COL32(255,255,255,46);
            auto boraderColor = IM_COL32(128,128,128,255);
            drawList->AddRectFilled(startpos, curpos, fillColor);
            drawList->AddRect(startpos, curpos, fillColor, 0.0f, 0, 1);
        }
    }
} // namespace pulsared
