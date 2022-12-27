#pragma once
#include <ApatiteEd/Assembly.h>
#include <CoreLib/Delegate.h>
#include "MenuContext.h"

namespace apatiteed
{
    using MenuCanOperate = FunctionDelegate<bool, const MenuContexts&>;
    using MenuAction = FunctionDelegate<void, const MenuContexts&>;
    using MenuCheckAction = FunctionDelegate<void, const MenuContexts&, bool>;

    class MenuEntry : public Object
    {
        CORELIB_DEF_TYPE(AssemblyObject_ApatiteEd, apatiteed::MenuEntry, Object);
    public:
        int priority;
        string name;

        virtual ~MenuEntry() override {}
    };
    CORELIB_DECL_SHORTSPTR(MenuEntry);

    class MenuEntrySubMenu : public MenuEntry
    {
        CORELIB_DEF_TYPE(AssemblyObject_ApatiteEd, apatiteed::MenuEntrySubMenu, MenuEntry);
    public:
        array_list<MenuEntry_sp> sub_menus;
    };
    CORELIB_DECL_SHORTSPTR(MenuEntrySubMenu);

    class MenuEntryButton : public MenuEntry
    {
        CORELIB_DEF_TYPE(AssemblyObject_ApatiteEd, apatiteed::MenuEntryButton, MenuEntry);
    public:
        sptr<MenuAction> action;
    };
    CORELIB_DECL_SHORTSPTR(MenuEntryButton);

    class MenuEntryCheck : public MenuEntry
    {
        CORELIB_DEF_TYPE(AssemblyObject_ApatiteEd, apatiteed::MenuEntryCheck, MenuEntry);
    public:
        bool is_check;
        sptr<MenuCheckAction> check_action;
    };
    CORELIB_DECL_SHORTSPTR(MenuEntryCheck);

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