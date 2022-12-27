#include <ApatiteEd/Menus/Menu.h>
#include <CoreLib/File.h>

namespace apatiteed
{

    static array_list<Menu*> menus;


    Menu* MenuManager::AddMenu(string_view menu_name)
    {
        auto menu = new Menu;
        menu->menu_name = menu_name;
        menus.push_back(menu);
        return menu;
    }

    void MenuManager::RemoveMenu(string_view menu_name)
    {
        auto it = std::find_if(menus.begin(), menus.end(), [&](Menu* menu) { return menu->menu_name == menu_name; });
        if (it != menus.end())
        {
            delete* it;
            menus.erase(it);
        }
    }

    Menu* MenuManager::GetMenu(string_view menu_name)
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

    const array_list<Menu*>& MenuManager::GetMenus()
    {
        return menus;
    }

}