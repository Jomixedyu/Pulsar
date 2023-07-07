#pragma once
#include "MenuEntry.h"
#include "ISubMenu.h"


namespace pulsared
{
    class MenuEntrySubMenu : public MenuEntry, public ISubMenu
    {
        CORELIB_DEF_TYPE(AssemblyObject_PulsarEd, pulsared::MenuEntrySubMenu, MenuEntry);
        CORELIB_IMPL_INTERFACES(ISubMenu);
    public:
        MenuEntrySubMenu(const string& name) : base(name), CORELIB_INIT_INTERFACE(ISubMenu)
        { }

        void AddEntry(MenuEntry_rsp entry)
        {
            this->entries.push_back(entry);
        }
        virtual const array_list<MenuEntry_sp>& GetEntries() override
        {
            return this->entries;
        }
        virtual MenuEntry_sp FindEntry(string_view name) override
        {
            for (auto& item : this->entries)
            {
                if (item->name == name)
                {
                    return item;
                }
            }
            return nullptr;
        }
    protected:
        array_list<MenuEntry_sp> entries;
    };
    CORELIB_DECL_SHORTSPTR(MenuEntrySubMenu);
}