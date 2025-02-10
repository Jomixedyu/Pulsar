#include "Windows/EditorWindowManager.h"
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

    static WaitableList<SPtr<EdGuiWindow>> _OpeningWindows;

    static SPtr<DockspaceWindow> _DockspaceWindow;
    static SPtr<StatusBarWindow> _StatusBarWindow;

    static void _ChangeWindowCheckedSatete(string_view window_name, bool check_state)
    {
        auto submenu = MenuManager::GetMainMenu()->FindSubMenuEntry("Window");
        submenu->FindCheckEntry(window_name)->IsChecked = check_state;
    }

    void EditorWindowManager::Initialize()
    {
        _DockspaceWindow = mksptr(new DockspaceWindow);
        // _StatusBarWindow = mksptr(new StatusBarWindow);
        _DockspaceWindow->Open();
        // _StatusBarWindow->Open();
    }

    void EditorWindowManager::Terminate()
    {
        _OpeningWindows.Refresh();
        for (auto& win : _OpeningWindows.items)
        {
            win->Close();
        }
        _OpeningWindows.Refresh();

        _DockspaceWindow.reset();
        _StatusBarWindow.reset();

    }


    void EditorWindowManager::Draw(float dt)
    {
        for (size_t i = 0; i < _OpeningWindows.items.size(); i++)
        {
            if (_OpeningWindows.items[i]->GetIsOpened())
            {
                _OpeningWindows.items[i]->DrawImGui(dt);
            }
        }
        _OpeningWindows.Refresh();
        GetEdApp()->GetTaskQueue().TickDraw();
    }

    EdGuiWindow_sp EditorWindowManager::GetOpeningWindow(string_view name)
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

    EdGuiWindow_sp EditorWindowManager::GetOpeningWindow(Type* type)
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

    array_list<EdGuiWindow_sp> EditorWindowManager::GetOpeningWindows(Type* type)
    {
        array_list<EdGuiWindow_sp> arr;
        for (auto& window : _OpeningWindows.items)
        {
            if (window->GetType() == type)
            {
                arr.push_back(window);
            }
        }
        return arr;
    }

    bool EditorWindowManager::RegisterOpeningWindow(EdGuiWindow_rsp window)
    {
        if (!window)
            return false;
        _OpeningWindows.Add(window);
        OnWindowStateChanged.Invoke(window.get(), true);
        return true;
    }

    void EditorWindowManager::UnregisterOpeningWindow(EdGuiWindow_rsp window)
    {
        if (!window)
            return;
        _OpeningWindows.Remove(window);
        OnWindowStateChanged.Invoke(window.get(), false);
    }
}
