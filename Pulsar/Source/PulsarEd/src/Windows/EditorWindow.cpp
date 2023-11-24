#include <PulsarEd/Windows/EditorWindow.h>
#include <PulsarEd/Menus/Menu.h>
#include <PulsarEd/Windows/EditorWindowManager.h>
#include <PulsarEd/Menus/MenuEntry.h>

namespace pulsared
{

    void EditorWindow::DrawImGui()
    {
        bool isOpened;
        ImGui::SetNextWindowSize(ImVec2{m_winSize.x, m_winSize.y});
        bool isDrawable = ImGui::Begin(StringUtil::Concat(GetWindowDisplayName(), "###", GetWindowName()).data(), &isOpened, GetGuiWindowFlags());
        if (isOpened)
        {
            m_isOpened = isOpened;
            if (isDrawable)
            {
                this->OnDrawImGui();
            }
            if (m_allowResize)
            {
                auto size = ImGui::GetWindowSize();
                m_winSize.x = size.x;
                m_winSize.y = size.y;
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
        if (this->m_isOpened)
            return true;

        if (!EditorWindowManager::RegisterOpeningWindow(self()))
        {
            return false;
        }
        this->m_isOpened = true;
        this->OnOpen();
        return true;
    }

    void EditorWindow::Close()
    {
        if (!this->m_isOpened)
            return;
        this->m_isOpened = false;
        this->OnClose();
        EditorWindowManager::UnregisterOpeningWindow(self());
    }

    static int _NewId()
    {
        static int id = 0;
        return ++id;
    }

    EditorWindow::EditorWindow()
        : m_windowId(_NewId()), m_allowResize(true), m_winSize(50, 50)
    {

    }
}
