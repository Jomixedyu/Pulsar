#pragma once
#include <ApatiteEd/Assembly.h>
#include "MenuEntry.h"

namespace apatiteed
{
    class ISubMenu : public IInterface
    {
        CORELIB_DEF_INTERFACE(AssemblyObject_ApatiteEd, apatiteed::ISubMenu, IInterface);
    public:
        virtual const array_list<MenuEntry_sp>& GetEntries() = 0;
        virtual MenuEntry_sp FindEntry(string_view name) = 0;
        sptr<class MenuEntrySubMenu> FindSubMenuEntry(string_view name);
        sptr<class MenuEntryCheck> FindCheckEntry(string_view name);
        sptr<class MenuEntryButton> FindButtonEntry(string_view name);
    };
}