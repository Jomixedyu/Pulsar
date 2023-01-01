#include <apatiteed/Windows/WorkspaceWindow.h>

namespace apatiteed
{
    void WorkspaceWindow::OnDrawImGui()
    {
        ImGui::InputText("Search", this->search_buf, sizeof(this->search_buf));
    }
    WorkspaceWindow::WorkspaceWindow()
    {
        ::memset(this->search_buf, 0, sizeof(this->search_buf));
    }
}