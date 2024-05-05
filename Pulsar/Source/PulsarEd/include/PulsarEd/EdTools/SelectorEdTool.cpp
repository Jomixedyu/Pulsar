#include "SelectorEdTool.h"

#include "EditorWorld.h"

#include <imgui/imgui.h>

namespace pulsared
{

    void SelectorEdTool::Begin()
    {
        base::Begin();
        m_frameSelector = false;
        Logger::Log("SelectorTool");
    }

    void SelectorEdTool::Tick(float dt)
    {
        base::Tick(dt);

        if (m_frameSelector)
        {
            ImDrawList* drawList = ImGui::GetWindowDrawList();
            auto curpos = ImGui::GetMousePos();
            auto startpos = ImVec2{m_frameSelectorStartPos.x, m_frameSelectorStartPos.y};
            auto fillColor = IM_COL32(255, 255, 255, 46);
            auto boraderColor = IM_COL32(128, 128, 128, 255);
            drawList->AddRectFilled(startpos, curpos, fillColor);
            drawList->AddRect(startpos, curpos, fillColor, 0.0f, 0, 1);
        }
    }
    void SelectorEdTool::OnMouseDown(const MouseEventData& e)
    {
        base::OnMouseDown(e);
        if (ImGui::IsWindowHovered() && e.ButtonId == ImGuiMouseButton_Left && m_enableSelect)
        {
            m_frameSelector = true;
            auto startpos = ImGui::GetMousePos();
            m_frameSelectorStartPos = {startpos.x, startpos.y};
        }
    }
    void SelectorEdTool::OnMouseUp(const MouseEventData& e)
    {
        base::OnMouseUp(e);
        if (e.ButtonId == ImGuiMouseButton_Left)
        {
            m_frameSelector = false;
        }
    }
    SelectionSet<Node>& SelectorEdTool::GetSelection()
    {
        return m_world->GetSelection();
    }
} // namespace pulsared
