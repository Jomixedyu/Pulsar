#pragma once
#include <ApatiteEd/Assembly.h>
#include <CoreLib/Delegate.h>
#include "MenuContext.h"
#include "MenuEntry.h"

namespace apatiteed
{
    class Menu
    {
    public:
        void AddEntry(MenuEntry_rsp entry)
        {
            this->entries.push_back(entry);
        }
        const array_list<MenuEntry_sp>& GetEntries()
        {
            return this->entries;
        }
        string GetMenuName()
        {
            return this->menu_name;
        }

        string menu_name;
    protected:
        array_list<MenuEntry_sp> entries;
    };


    class MenuManager
    {
    public:
        static Menu* AddMenu(string_view menu_name);
        static void RemoveMenu(string_view menu_name);
        static Menu* GetMenu(string_view menu_name);
        static const array_list<Menu*>& GetMenus();
    };
}