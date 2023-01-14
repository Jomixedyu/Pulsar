#include "EditorWindowManager.h"
#include "SceneWindow.h"
#include "WorkspaceWindow.h"
#include "PropertiesWindow.h"
#include "ConsoleWindow.h"
#include "OutlinerWindow.h"
#include "OutputWindow.h"
#include "MainMenuBarWindow.h"
#include "DockspaceWindow.h"
#include <ApatiteEd/Menus/_include.h>

#include <queue>
#include <tuple>

namespace apatiteed
{

    template<typename T>
    class WaitableList
    {
    public:
        enum QueueType { ADD, REMOVE };

        array_list<T> items;

        array_list<std::tuple<QueueType, T>> waitlist;

        void Add(const T& t)
        {
            waitlist.push_back({ QueueType::ADD, t });
        }
        void Remove(const T& t)
        {
            waitlist.push_back({ QueueType::REMOVE, t });
        }
        bool Contains(const T& t)
        {
            int size = (int)waitlist.size();
            for (int i = size - 1; i >= 0; i--)
            {
                auto& [type, value] = waitlist[i];
                if (value != t)
                {
                    continue;
                }
                if (type == QueueType::REMOVE)
                {
                    return false;
                }
                if (type == QueueType::ADD)
                {
                    return true;
                }
            }
            auto it = std::find(items.begin(), items.end(), t);
            return it != items.end();
        }

        void Refresh()
        {
            decltype(waitlist) list;
            list.swap(waitlist);

            for (auto& [type, t] : list)
            {
                if (type == QueueType::ADD)
                {
                    items.push_back(t);
                }
                else
                {
                    auto it = std::find(items.begin(), items.end(), t);
                    if (it != items.end())
                    {
                        items.erase(it);
                    }
                }
            }
        }
    };

    static WaitableList<sptr<EditorWindow>> _registered_windows;

    static map<string, Type*> _registered_menu;

    static sptr<DockspaceWindow> _DockspaceWindow;
    static sptr<MainMenuBarWindow> _MainMenuBarWindow;

    static bool _HasRegistered(const sptr<EditorWindow>& win)
    {
        return _registered_windows.Contains(win);
    }


    static bool _HasRegisterMenu(Type* type)
    {
        auto menu_it = std::find_if(_registered_menu.begin(), _registered_menu.end(), [&](auto& a) { return a.second == type; });
        if (menu_it == _registered_menu.end())
        {
            return false;
        }
        return true;
    }

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

    static void _ChangeWindowCheckedSatete(string_view window_name, bool check_state)
    {
        auto submenu = MenuManager::GetMainMenu()->FindSubMenuEntry("Window");
        submenu->FindCheckEntry(window_name)->is_check = check_state;
    }


    void EditorWindowManager::Reset()
    {
        _DockspaceWindow = mksptr(new DockspaceWindow);
        _MainMenuBarWindow = mksptr(new MainMenuBarWindow);
        _DockspaceWindow->Open();
        _MainMenuBarWindow->Open();



        _registered_menu.emplace(WorkspaceWindow::StaticWindowName(), cltypeof<WorkspaceWindow>());
        _registered_menu.emplace(PropertiesWindow::StaticWindowName(), cltypeof<PropertiesWindow>());
        _registered_menu.emplace(ConsoleWindow::StaticWindowName(), cltypeof<ConsoleWindow>());
        _registered_menu.emplace(OutlinerWindow::StaticWindowName(), cltypeof<OutlinerWindow>());
        _registered_menu.emplace(SceneWindow::StaticWindowName(), cltypeof<SceneWindow>());
        _registered_menu.emplace(OutputWindow::StaticWindowName(), cltypeof<OutputWindow>());

        auto main_menu = MenuManager::GetMainMenu();
        assert(main_menu);
        auto win_menu = main_menu->FindSubMenuEntry("Window");
        assert(win_menu);

        auto check_action = MenuCheckAction::FromRaw(_CheckableLabelAction);

        for (auto& [name, type] : _registered_menu)
        {
            win_menu->AddEntry(mksptr(new MenuEntryCheck{ string{ name }, false, check_action }));
            //force show all
            sptr_cast<EditorWindow>(type->CreateSharedInstance({}))->Open();
        }

    }


    void EditorWindowManager::Draw()
    {
        for (size_t i = 0; i < _registered_windows.items.size(); i++)
        {
            auto& item = _registered_windows.items[i];
            item->DrawImGui();
        }
        _registered_windows.Refresh();
    }

    EditorWindow_sp EditorWindowManager::GetWindow(string_view name)
    {
        for (auto& window : _registered_windows.items)
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
        for (auto& window : _registered_windows.items)
        {
            if (window->GetType() == type)
            {
                if (_HasRegistered(window))
                    return window;
            }
        }
        return nullptr;
    }

    bool EditorWindowManager::RegisterWindow(EditorWindow_rsp window)
    {
        if (!window) return false;
        if (_HasRegisterMenu(window->GetType()))
        {
            if (_HasRegistered(window))
            {
                return false;
            }
            //change menu check
            _ChangeWindowCheckedSatete(window->GetWindowName(), true);
        }

        _registered_windows.Add(window);
        return true;
    }
    void EditorWindowManager::UnRegisterWindow(EditorWindow_rsp window)
    {
        if (!window) return;
        if (_HasRegisterMenu(window->GetType()))
        {
            if (!_HasRegistered(window))
            {
                return;
            }
            //change menu check
            _ChangeWindowCheckedSatete(window->GetWindowName(), false);
        }

        _registered_windows.Remove(window);
    }
}