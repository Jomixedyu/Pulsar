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
    static array_list<sptr<EditorWindow>> _registered_windows;
    static array_list<sptr<EditorWindow>> _registered_windows_wait_del;

    static map<string, Type*> _registered_menu;

    static sptr<DockspaceWindow> _DockspaceWindow;
    static sptr<MainMenuBarWindow> _MainMenuBarWindow;

    static void _CheckableLabelAction(MenuContexts_sp ctx, bool checked)
    {
        if (!ctx)
        {
            return;
        }

        if (checked)
        {
            auto it = _registered_menu.find(ctx->entry_name);
            if (it == _registered_menu.end())
            {
                return;
            }

            auto ptr = sptr_cast<EditorWindow>(it->second->CreateSharedInstance({}));
            assert(ptr);
            ptr->Open();
        }
        else
        {
            if (auto win = EditorWindowManager::GetWindow(ctx->entry_name))
            {
                win->Close();
            }
        }

    }



    void EditorWindowManager::Reset()
    {
        _registered_menu.emplace(ProjectWindow::StaticWindowName(), cltypeof<ProjectWindow>());
        _registered_menu.emplace(PropertiesWindow::StaticWindowName(), cltypeof<PropertiesWindow>());
        _registered_menu.emplace(ConsoleWindow::StaticWindowName(), cltypeof<ConsoleWindow>());
        _registered_menu.emplace(OutlinerWindow::StaticWindowName(), cltypeof<OutlinerWindow>());
        _registered_menu.emplace(SceneWindow::StaticWindowName(), cltypeof<SceneWindow>());
        _registered_menu.emplace(OutputWindow::StaticWindowName(), cltypeof<OutputWindow>());


        auto main_menu = MenuManager::GetOrAddMenu("Main");
        assert(main_menu);
        auto win_menu = main_menu->FindMenuEntry<MenuEntrySubMenu>("Window");
        assert(win_menu);

        auto check_action = MenuCheckAction::FromRaw(_CheckableLabelAction);

        for (auto& [name, type] : _registered_menu)
        {
            win_menu->AddEntry(mksptr(new MenuEntryCheck{ string{ name }, false, check_action }));
        }

        _DockspaceWindow = mksptr(new DockspaceWindow);
        _MainMenuBarWindow = mksptr(new MainMenuBarWindow);
        _DockspaceWindow->Open();
        _MainMenuBarWindow->Open();
    }


    void EditorWindowManager::Draw()
    {
        for (size_t i = 0; i < _registered_windows.size(); i++)
        {
            auto& item = _registered_windows[i];
            auto it = std::find(_registered_windows_wait_del.begin(), _registered_windows_wait_del.end(), item);
            if (it == _registered_windows_wait_del.end())
            {
                item->DrawImGui();
            }
        }

        for (auto& window : _registered_windows_wait_del)
        {
            auto it = std::find(_registered_windows.begin(), _registered_windows.end(), window);
            if (it != _registered_windows.end())
            {
                _registered_windows.erase(it);
            }
        }
    }

    const std::vector<EditorWindow_sp>& EditorWindowManager::GetWindows()
    {
        return _registered_windows;
    }

    EditorWindow_sp EditorWindowManager::GetWindow(string_view name)
    {
        for (auto& window : _registered_windows)
        {
            if (window->GetWindowName() == name)
            {
                return window;
            }
        }
        return nullptr;
    }

    EditorWindow_sp EditorWindowManager::GetWindow(Type* type)
    {
        for (auto& window : _registered_windows)
        {
            if (window->GetType() == type)
            {
                return window;
            }
        }
        return nullptr;
    }

    void EditorWindowManager::RegisterWindow(EditorWindow_rsp window)
    {
        if (!window) return;
        auto it = std::find(_registered_windows.begin(), _registered_windows.end(), window);
        if (it == _registered_windows.end())
            _registered_windows.push_back(window);
    }
    void EditorWindowManager::UnRegisterWindow(EditorWindow_rsp window)
    {
        if (!window) return;
        auto it = std::find(_registered_windows.begin(), _registered_windows.end(), window);
        if (it != _registered_windows.end())
        {
            _registered_windows_wait_del.push_back(window);
        }
    }
}