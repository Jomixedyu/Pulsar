#include <PulsarEd/Menus/Menu.h>
#include <PulsarEd/Menus/MenuEntry.h>
#include <PulsarEd/Windows/EdGuiWindow.h>
#include <PulsarEd/Windows/EditorWindowManager.h>

namespace pulsared
{

    void EdGuiWindow::DrawImGui(float dt)
    {
        bool isOpened = true;
        ImGui::SetNextWindowSize(ImVec2{m_winSize.x, m_winSize.y});
        const auto winName = StringUtil::Concat(GetWindowDisplayName(), "###", GetWindowName());

        auto winClass = GetGuiWindowClass();
        ImGui::SetNextWindowClass(&winClass);

        const bool isDrawable = ImGui::Begin(winName.c_str(), &isOpened, GetGuiWindowFlags());
        if (isOpened)
        {
            m_isOpened = isOpened;
            if (isDrawable)
            {
                this->OnDrawImGui(dt);
            }
            if (m_allowResize)
            {
                const auto size = ImGui::GetWindowSize();
                m_winSize.x = size.x;
                m_winSize.y = size.y;
            }
        }
        else
        {
            this->Close();
        }

        ImGui::End();
    }

    void EdGuiWindow::OnDrawImGui(float dt)
    {

    }

    bool EdGuiWindow::Open()
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

    void EdGuiWindow::Close()
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

    EdGuiWindow::EdGuiWindow()
        : m_windowId(_NewId()), m_allowResize(true), m_winSize(50, 50)
    {

    }
}
