#include <PulsarEd/Menus/Menu.h>
#include <CoreLib/File.h>

namespace pulsared
{

    static array_list<Menu_sp> menus;


    Menu_sp MenuManager::GetOrAddMenu(string_view menu_name)
    {
        if (auto menu = GetMenu(menu_name))
        {
            return menu;
        }
        return AddMenu(menu_name);
    }

    Menu_sp MenuManager::AddMenu(string_view menu_name)
    {
        auto menu = mksptr(new Menu);
        menu->menu_name = menu_name;
        menus.push_back(menu);
        return menu;
    }

    void MenuManager::RemoveMenu(string_view menu_name)
    {
        auto it = std::find_if(menus.begin(), menus.end(), [&](Menu_sp menu) { return menu->menu_name == menu_name; });
        if (it != menus.end())
        {
            menus.erase(it);
        }
    }

    Menu_sp MenuManager::GetMenu(string_view menu_name)
    {
        for (auto& item : menus)
        {
            if (item->menu_name == menu_name)
            {
                return item;
            }
        }
        return nullptr;
    }

    Menu_sp MenuManager::GetMainMenu()
    {
        return GetOrAddMenu("Main");
    }

    const array_list<Menu_sp>& MenuManager::GetMenus()
    {
        return menus;
    }

    MenuEntry_sp Menu::FindEntry(string_view name)
    {
        for (auto& item : this->entries)
        {
            if (item->Name == name)
            {
                return item;
            }
        }
        return nullptr;
    }
    void Menu::RemoveEntry(string_view name)
    {
        auto it = std::find_if(entries.begin(), entries.end(), [name](auto& entry) {
            return entry->Name == name;
            });
        if (it != this->entries.end())
        {
            this->entries.erase(it);
        }
    }
}