#include "Menus/MenuEntrySubMenu.h"
#include <PulsarEd/Menus/Menu.h>

#include <CoreLib/File.h>
#include <ranges>

namespace pulsared
{

    static auto& menus()
    {
        static array_list<MenuEntrySubMenu_sp> value;
        return value;
    }


    ISubMenu* MenuManager::GetOrAddMenu(string_view menu_name)
    {
        if (auto menu = GetMenu(menu_name))
        {
            return menu;
        }
        return AddMenu(menu_name);
    }

    ISubMenu* MenuManager::AddMenu(string_view menu_name)
    {
        auto menu = mksptr(new MenuEntrySubMenu(string{menu_name}));
        menus().push_back(menu);
        return menu.get();
    }

    void MenuManager::RemoveMenu(string_view menu_name)
    {
        auto it = std::ranges::find_if(menus(), [&](const MenuEntrySubMenu_sp& menu) { return menu->GetMenuName() == menu_name; });
        if (it != menus().end())
        {
            menus().erase(it);
        }
    }

    ISubMenu* MenuManager::GetMenu(string_view menu_name)
    {
        for (auto& item : menus())
        {
            if (item->Name == menu_name)
            {
                return item.get();
            }
        }
        return nullptr;
    }

    ISubMenu* MenuManager::GetMainMenu()
    {
        return GetOrAddMenu("Main");
    }

    array_list<ISubMenu*> MenuManager::GetMenus()
    {
        array_list<ISubMenu*> menu;
        for (const auto& element : menus())
        {
            menu.push_back(element.get());
        }
        return menu;
    }

    static auto& contexts()
    {
        static hash_map<string, array_list<SPtr<OnGetContext>>> values;
        return values;
    }
    void MenuManager::RegisterContextProvider(string_view name, const SPtr<OnGetContext>& callback)
    {
        contexts()[string(name)].push_back(callback);
    }
    void MenuManager::UnregisterContextProvider(string_view name, const SPtr<OnGetContext>& callback)
    {
        auto& values = contexts()[string(name)];
        const auto it = std::ranges::find(values, callback);
        if(it != values.end())
        {
            values.erase(it);
        }
    }
    MenuContexts_sp MenuManager::RequestContexts(string_view name)
    {
        auto context = mksptr(new MenuContexts);
        for (const auto& func : contexts()[string(name)])
        {
            if (func->IsValid())
            {
                context->Contexts.push_back(func->Invoke());
            }
        }
        return context;
    }

}