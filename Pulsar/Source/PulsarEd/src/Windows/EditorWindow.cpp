#include <PulsarEd/Windows/EditorWindow.h>
#include <PulsarEd/Menus/Menu.h>
#include <PulsarEd/Windows/EditorWindowManager.h>
#include <PulsarEd/Menus/MenuEntry.h>

namespace pulsared
{
    string_view EditorWindow::GetWindowName() const
    {
        return "editor window";
    }

    void EditorWindow::DrawImGui()
    {
        bool isOpened;
        bool isDrawable = ImGui::Begin(GetWindowName().data(), &isOpened, GetGuiWindowFlags());
        if (isOpened)
        {
            m_isOpened = isOpened;
            if (isDrawable)
            {
                this->OnDrawImGui();
            }
        }
        else
        {
            //close
            this->Close();
        }

        ImGui::End();

    }

    void EditorWindow::OnDrawImGui()
    {

    }

    bool EditorWindow::Open()
    {
        if (this->m_isOpened) return true;
        
        if (!EditorWindowManager::RegisterWindow(self()))
        {
            return false;
        }
        this->m_isOpened = true;
        this->OnOpen();
        return true;
    }
    void EditorWindow::Close()
    {
        if (!this->m_isOpened) return;
        this->m_isOpened = false;
        this->OnClose();
        EditorWindowManager::UnRegisterWindow(self());
    }

    static int _NewId()
    {
        static int id = 0;
        return ++id;
    }

    EditorWindow::EditorWindow() : m_windowId(_NewId())
    {

    }
}