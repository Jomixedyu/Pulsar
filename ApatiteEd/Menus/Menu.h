#pragma once
#include <ApatiteEd/Assembly.h>
#include <CoreLib/Delegate.h>
#include "MenuContext.h"
#include "MenuEntry.h"
#include "ISubMenu.h"

namespace apatiteed
{
    class Menu : public Object, public ISubMenu
    {
        CORELIB_DEF_TYPE(AssemblyObject_ApatiteEd, apatiteed::Menu, Object);
        CORELIB_IMPL_INTERFACES(ISubMenu);
    public:
        Menu() : CORELIB_INIT_INTERFACE(ISubMenu)
        {

        }
        void AddEntry(MenuEntry_rsp entry)
        {
            this->entries.push_back(entry);
        }
        virtual const array_list<MenuEntry_sp>& GetEntries() override
        {
            return this->entries;
        }
        string GetMenuName()
        {
            return this->menu_name;
        }

        virtual MenuEntry_sp FindEntry(string_view name) override;

        template<typename T>
        sptr<T> FindMenuEntry(string_view name)
        {
            return sptr_cast<T>(this->FindEntry(name));
        }

       
    public:
        string menu_name;
    protected:
        array_list<MenuEntry_sp> entries;


    };
    CORELIB_DECL_SHORTSPTR(Menu);



    class MenuManager
    {
    public:
        static Menu_sp GetOrAddMenu(string_view menu_name);
        static Menu_sp AddMenu(string_view menu_name);
        static void RemoveMenu(string_view menu_name);
        static Menu_sp GetMenu(string_view menu_name);
        static Menu_sp GetMainMenu();
        static const array_list<Menu_sp>& GetMenus();
    };
}