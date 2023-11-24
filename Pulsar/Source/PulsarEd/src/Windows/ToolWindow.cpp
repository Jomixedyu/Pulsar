#include <PulsarEd/Windows/ToolWindow.h>

namespace pulsared
{
    void ToolWindow::DrawImGui()
    {
        if (!m_inited)
        {
            m_inited = true;
            ImGui::SetNextWindowSize({ 400,600 });
        }
        base::DrawImGui();
    }
}