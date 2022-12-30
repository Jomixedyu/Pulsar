#include "EditorWindowManager.h"
#include "SceneWindow.h"
#include "ProjectWindow.h"
#include "PropertiesWindow.h"
#include "ConsoleWindow.h"
#include "OutlinerWindow.h"
#include "OutputWindow.h"
#include "MainMenuBarWindow.h"
#include "DockspaceWindow.h"
#include <ApatiteEd/Menus/Menu.h>

namespace apatiteed
{
    static std::vector<EditorWindow*> _windows;

    void EditorWindowManager::Reset()
    {
        _windows.push_back((new DockspaceWindow));
        _windows.push_back((new MainMenuBarWindow));
        _windows.push_back((new ProjectWindow));
        _windows.push_back((new PropertiesWindow));
        _windows.push_back((new ConsoleWindow));
        _windows.push_back((new OutlinerWindow));
        _windows.push_back((new SceneWindow));
        _windows.push_back((new OutputWindow));

        for (auto window : _windows)
        {
            window->Open();
        }

        auto main_menu = MenuManager::GetOrAddMenu("Main");
        auto win_menu = main_menu->FindMenuEntry<MenuEntrySubMenu>("Window");

        auto check_action = MenuCheckAction::FromLambda(
            [](MenuContexts_sp ctx, bool checked) 
            {
                if (!ctx) return;
                auto win = GetWindow(ctx->entry_name);
                if (!win) return;
                if (checked)
                {
                    win->Open();
                }
                else
                {
                    win->Close();
                }
            }
        );

        for (auto& window : _windows)
        {
            if (!window->get_is_register_menu())
                continue;
            win_menu->AddEntry(mksptr(new MenuEntryCheck{ string{ window->GetWindowName() }, window->get_is_opened(), check_action}));
        }

    }

    void EditorWindowManager::Draw()
    {
        for (const auto& window : _windows)
        {
            if (!window->get_is_opened())
            {
                continue;
            }
            window->DrawImGui();
        }
    }

    const std::vector<EditorWindow*>& EditorWindowManager::GetWindows()
    {
        return _windows;
    }
    EditorWindow* EditorWindowManager::GetWindow(string_view name)
    {
        for (auto& window : _windows)
        {
            if (window->GetWindowName() == name)
            {
                return window;
            }
        }
        return nullptr;
    }
}