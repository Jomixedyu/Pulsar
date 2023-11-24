#include "Windows/EditorWindowManager.h"
#include "Windows/SceneWindow.h"
#include "Windows/WorkspaceWindow.h"
#include "Windows/PropertiesWindow.h"
#include "Windows/ConsoleWindow.h"
#include "Windows/OutlinerWindow.h"
#include "Windows/OutputWindow.h"
#include "Windows/MainMenuBarWindow.h"
#include "Windows/DockspaceWindow.h"
#include "Windows/StatusBarWindow.h"
#include <PulsarEd/Menus/Types.h>
#include "ExclusiveTask.h"
#include "EditorAppInstance.h"
#include <queue>
#include <tuple>

namespace pulsared
{

    template <typename T>
    class WaitableList
    {
    public:
        enum QueueType { ADD, REMOVE };

        array_list<T> items;

        array_list<std::tuple<QueueType, T>> waitlist;

        void Add(const T& t)
        {
            waitlist.push_back({QueueType::ADD, t});
        }

        void Remove(const T& t)
        {
            waitlist.push_back({QueueType::REMOVE, t});
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

    static WaitableList<sptr<EditorWindow>> _OpeningWindows;

    static array_list<EditorWindow_sp> _PanelWindows;

    static sptr<DockspaceWindow> _DockspaceWindow;
    static sptr<MainMenuBarWindow> _MainMenuBarWindow;
    static sptr<StatusBarWindow> _StatusBarWindow;

    static void _ChangeWindowCheckedSatete(string_view window_name, bool check_state)
    {
        auto submenu = MenuManager::GetMainMenu()->FindSubMenuEntry("Window");
        submenu->FindCheckEntry(window_name)->IsChecked = check_state;
    }

    void EditorWindowManager::Initialize()
    {
        _DockspaceWindow = mksptr(new DockspaceWindow);
        _MainMenuBarWindow = mksptr(new MainMenuBarWindow);
        _StatusBarWindow = mksptr(new StatusBarWindow);
        _DockspaceWindow->Open();
        _MainMenuBarWindow->Open();
        _StatusBarWindow->Open();
    }

    void EditorWindowManager::Terminate()
    {
        _OpeningWindows.Refresh();
        for (auto& win : _OpeningWindows.items)
        {
            win->Close();
        }
        _OpeningWindows.items.clear();

        _DockspaceWindow.reset();
        _MainMenuBarWindow.reset();
        _StatusBarWindow.reset();

    }


    void EditorWindowManager::Draw()
    {
        for (size_t i = 0; i < _OpeningWindows.items.size(); i++)
        {
            if (_OpeningWindows.items[i]->GetIsOpened())
            {
                _OpeningWindows.items[i]->DrawImGui();
            }
        }
        _OpeningWindows.Refresh();
        GetEdApp()->GetTaskQueue().TickDraw();
    }

    EditorWindow_sp EditorWindowManager::GetOpeningWindow(string_view name)
    {
        for (auto& window : _OpeningWindows.items)
        {
            if (window->GetWindowName() == name)
            {
                return window;
            }
        }
        return nullptr;
    }

    EditorWindow_sp EditorWindowManager::GetOpeningWindow(Type* type)
    {
        for (auto& window : _OpeningWindows.items)
        {
            if (window->GetType() == type)
            {
                return window;
            }
        }
        return nullptr;
    }

    array_list<EditorWindow_sp> EditorWindowManager::GetOpeningWindows(Type* type)
    {
        array_list<EditorWindow_sp> arr;
        for (auto& window : _OpeningWindows.items)
        {
            if (window->GetType() == type)
            {
                arr.push_back(window);
            }
        }
        return arr;
    }

    bool EditorWindowManager::RegisterOpeningWindow(EditorWindow_rsp window)
    {
        if (!window)
            return false;
        _OpeningWindows.Add(window);
        return true;
    }

    void EditorWindowManager::UnregisterOpeningWindow(EditorWindow_rsp window)
    {
        if (!window)
            return;
        _OpeningWindows.Remove(window);
    }

    static void _CheckableLabelAction(sptr<MenuContexts> ctxs, bool checked)
    {
        auto win = EditorWindowManager::GetPanelWindow(ctxs->EntryName);
        if (checked)
        {
            win->Open();
        }
        else
        {
            win->Close();
        }
    }

    void EditorWindowManager::RegisterPanelWindowType(Type* type)
    {
        auto panel = sptr_cast<EditorWindow>(type->CreateSharedInstance({}));
        _PanelWindows.push_back(panel);

        string displayName = string{panel->GetWindowDisplayName()};

        auto checkEntry = new MenuEntryCheck(type->GetName(), displayName, MenuCheckAction::FromRaw(_CheckableLabelAction));
        auto window = MenuManager::GetMainMenu()->FindSubMenuEntry("Window");
        window->AddEntry(mksptr(checkEntry));
    }

    void EditorWindowManager::UnregisterPanelWindowType(Type* type)
    {
        auto it = std::find_if(_PanelWindows.begin(), _PanelWindows.end(), [type](EditorWindow_rsp win) {
            return win->GetType() == type;
        });
        if (it != _PanelWindows.end())
        {
            _PanelWindows.erase(it);

            auto window = MenuManager::GetMainMenu()->FindSubMenuEntry("Window");
            window->RemoveEntry(type->GetName());
        }
    }

    EditorWindow_sp EditorWindowManager::GetPanelWindow(Type* type)
    {
        for (auto& win : _PanelWindows)
        {
            if (win->GetType() == type)
            {
                return win;
            }
        }
        return nullptr;
    }

    EditorWindow_sp EditorWindowManager::GetPanelWindow(string_view name)
    {
        for (auto& win : _PanelWindows)
        {
            if (win->GetWindowName() == name)
            {
                return win;
            }
        }
        return nullptr;
    }
}
