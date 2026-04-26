#include <PulsarEd/Windows/ToolWindow.h>

namespace pulsared
{
    ToolWindow::ToolWindow()
    {
        m_winSize = {400, 700};
    }
    string ToolWindow::GetWindowName() const
    {
        return std::to_string(GetWindowId());
    }

    void ToolWindow::DrawImGui(float dt)
    {

        base::DrawImGui(dt);
    }
} // namespace pulsared