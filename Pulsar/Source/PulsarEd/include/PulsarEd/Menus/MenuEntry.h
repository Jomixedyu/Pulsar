#pragma once
#include <CoreLib/Core.h>
#include <CoreLib/Delegate.h>
#include "MenuContext.h"

namespace pulsared
{
    using MenuCanOperate = FunctionDelegate<bool, sptr<MenuContexts>>;
    using MenuAction = FunctionDelegate<void, sptr<MenuContexts>>;
    using MenuCheckAction = FunctionDelegate<void, sptr<MenuContexts>, bool>;

    class MenuEntry : public Object
    {
        CORELIB_DEF_TYPE(AssemblyObject_PulsarEd, pulsared::MenuEntry, Object);
    public:
        int priority;
        string name;

        MenuEntry(const string& name) : name(name), priority(9999) {}

        virtual ~MenuEntry() override {}
    };
    CORELIB_DECL_SHORTSPTR(MenuEntry);



    class MenuEntryButton : public MenuEntry
    {
        CORELIB_DEF_TYPE(AssemblyObject_PulsarEd, pulsared::MenuEntryButton, MenuEntry);
    public:
        using base::base;

        sptr<MenuAction> action;
    };
    CORELIB_DECL_SHORTSPTR(MenuEntryButton);



    class MenuEntryCheck : public MenuEntry
    {
        CORELIB_DEF_TYPE(AssemblyObject_PulsarEd, pulsared::MenuEntryCheck, MenuEntry);
    public:
        MenuEntryCheck(const string& name, bool is_check, const sptr<MenuCheckAction>& check_action)
            : base(name), is_check(is_check), check_action(check_action)
        { }

        bool is_check;
        sptr<MenuCheckAction> check_action;
    };
    CORELIB_DECL_SHORTSPTR(MenuEntryCheck);

}