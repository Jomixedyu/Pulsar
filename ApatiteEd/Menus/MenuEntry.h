#pragma once
#include <CoreLib/Core.h>
#include <CoreLib/Delegate.h>

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

        void AddEntry(MenuEntry_rsp entry)
        {
            this->entries.push_back(entry);
        }
        const array_list<MenuEntry_sp>& GetEntries()
        {
            return this->entries;
        }
    protected:
        array_list<MenuEntry_sp> entries;
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
        MenuEntryCheck(bool is_check, const sptr<MenuCheckAction>& check_action)
            : is_check(is_check), check_action(check_action)
        { }

        bool is_check;
        sptr<MenuCheckAction> check_action;
    };
    CORELIB_DECL_SHORTSPTR(MenuEntryCheck);

}