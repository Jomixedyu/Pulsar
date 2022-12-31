#include <ApatiteEd/Windows/EditorWindow.h>
#include <ApatiteEd/Menus/Menu.h>
#include <ApatiteEd/Windows/EditorWindowManager.h>

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
        EditorWindowManager::RegisterWindow(self());
        this->OnOpen();
    }
    void EditorWindow::Close()
    {
        if (!this->is_opened) return;
        this->is_opened = false;
        this->OnClose();
        EditorWindowManager::UnRegisterWindow(self());
    }

    static int _NewId()
    {
        static int id = 0;
        return ++id;
    }

    EditorWindow::EditorWindow() : window_id_(_NewId())
    {

    }
}