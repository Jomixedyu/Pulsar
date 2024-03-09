#pragma once
#include "MenuEntry.h"
#include "ISubMenu.h"


namespace pulsared
{
    class MenuEntrySubMenu : public MenuEntry, public ISubMenu
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::MenuEntrySubMenu, MenuEntry);
        CORELIB_IMPL_INTERFACES(ISubMenu);
    public:
        explicit MenuEntrySubMenu(const string& name) : base(name), CORELIB_INIT_INTERFACE(ISubMenu)
        { }
        MenuEntrySubMenu(const string& name, const string& displayName)
            : base(name, displayName), CORELIB_INIT_INTERFACE(ISubMenu)
        { }
        virtual string GetMenuName() const override
        {
            return Name;
        }
        virtual void AddEntry(MenuEntry_rsp entry) override
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
                if (item->Name == name)
                {
                    return item;
                }
            }
            return nullptr;
        }
        virtual void RemoveEntry(string_view name) override
        {
            auto it = std::find_if(entries.begin(), entries.end(), [name](auto& entry) {
                return entry->Name == name;
                });
            if (it != this->entries.end())
            {
                this->entries.erase(it);
            }
        }

        SPtr<MenuCanVisibility> Visibility;
    protected:
        array_list<MenuEntry_sp> entries;
    };
    CORELIB_DECL_SHORTSPTR(MenuEntrySubMenu);
}