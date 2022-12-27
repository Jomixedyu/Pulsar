#include <ApatiteEd/Menus/Menu.h>
#include <CoreLib/File.h>

namespace apatiteed
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

    const array_list<Menu_sp>& MenuManager::GetMenus()
    {
        return menus;
    }

}