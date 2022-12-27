#include <ApatiteEd/Windows/EditorWindow.h>
#include <ApatiteEd/Menus/Menu.h>

namespace apatiteed
{
    string_view EditorWindow::GetWindowName() const
    {
        return "editor window";
    }

    void EditorWindow::DrawImGui()
    {
        ImGui::Begin(GetWindowName().data(), &is_opened, GetGuiWindowFlags());
        this->OnDrawImGui();
        ImGui::End();
    }

    void EditorWindow::OnDrawImGui()
    {

    }

    void EditorWindow::Open()
    {
        if (this->is_opened) return;
        this->is_opened = true;
        this->OnOpen();
    }
    void EditorWindow::Close()
    {
        if (!this->is_opened) return;
        this->is_opened = false;
        this->OnClose();
    }
    EditorWindow::EditorWindow()
    {

    }
}