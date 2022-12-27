#pragma once
#include <CoreLib/Core.h>
#include <CoreLib/Delegate.h>

namespace apatiteed
{
    using MenuCanOperate = FunctionDelegate<bool, sptr<MenuContexts>>;
    using MenuAction = FunctionDelegate<void, sptr<MenuContexts>>;
    using MenuCheckAction = FunctionDelegate<void, sptr<MenuContexts>, bool>;



    class MenuEntry : public Object
    {
        CORELIB_DEF_TYPE(AssemblyObject_ApatiteEd, apatiteed::MenuEntry, Object);
    public:
        int priority;
        string name;

        MenuEntry(const string& name) : name(name) {}

        virtual ~MenuEntry() override {}
    };
    CORELIB_DECL_SHORTSPTR(MenuEntry);

    class ISubMenu : public IInterface
    {
        CORELIB_DEF_INTERFACE(AssemblyObject_ApatiteEd, apatiteed::ISubMenu, IInterface);
    public:
        virtual const array_list<MenuEntry_sp>& GetEntries() = 0;
    };

    class MenuEntrySubMenu : public MenuEntry, public ISubMenu
    {
        CORELIB_DEF_TYPE(AssemblyObject_ApatiteEd, apatiteed::MenuEntrySubMenu, MenuEntry);
        CORELIB_IMPL_INTERFACES(ISubMenu);
    public:
        using base::base;

        void AddEntry(MenuEntry_rsp entry)
        {
            this->entries.push_back(entry);
        }
        virtual const array_list<MenuEntry_sp>& GetEntries() override
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
        MenuEntryCheck(const string& name, bool is_check, const sptr<MenuCheckAction>& check_action)
            : base(name), is_check(is_check), check_action(check_action)
        { }

        bool is_check;
        sptr<MenuCheckAction> check_action;
    };
    CORELIB_DECL_SHORTSPTR(MenuEntryCheck);

}