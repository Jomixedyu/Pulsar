#include "EditorWindowManager.h"
#include "SceneWindow.h"
#include "ProjectWindow.h"
#include "PropertiesWindow.h"
#include "ConsoleWindow.h"
#include "OutlinerWindow.h"
#include "OutputWindow.h"
#include "MainMenuBarWindow.h"
#include "DockspaceWindow.h"

namespace apatiteed
{
    void EditorWindowManager::Reset()
    {
        this->windows.push_back((new DockspaceWindow));
        this->windows.push_back((new MainMenuBarWindow));
        this->windows.push_back((new ProjectWindow));
        this->windows.push_back((new PropertiesWindow));
        this->windows.push_back((new ConsoleWindow));
        this->windows.push_back((new OutlinerWindow));
        this->windows.push_back((new SceneWindow));
        this->windows.push_back((new OutputWindow));

        for (auto window : this->windows)
        {
            window->Open();
        }
    }
    void EditorWindowManager::Draw()
    {
        for (const auto& window : windows)
        {
            if (!window->get_is_opened())
            {
                continue;
            }
            window->DrawImGui();
        }
    }
    const std::vector<EditorWindow*>& EditorWindowManager::GetWindows() const
    {
        return this->windows;
    }
}