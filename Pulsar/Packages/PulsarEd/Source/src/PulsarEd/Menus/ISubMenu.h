#pragma once
#include <PulsarEd/Assembly.h>
#include "MenuEntry.h"

namespace pulsared
{
    class ISubMenu : public IInterface
    {
        CORELIB_DEF_INTERFACE(AssemblyObject_pulsared, pulsared::ISubMenu, IInterface);
    public:
        virtual string GetMenuName() const = 0;
        virtual const array_list<MenuEntry_sp>& GetEntries() = 0;
        virtual void AddEntry(MenuEntry_rsp entry) = 0;
        virtual MenuEntry_sp FindEntry(string_view name) = 0;
        virtual void RemoveEntry(string_view name) = 0;
        SPtr<class MenuEntrySubMenu> FindSubMenuEntry(string_view name);
        SPtr<class MenuEntrySubMenu> FindOrNewMenuEntry(string_view name);
        SPtr<class MenuEntryCheck> FindCheckEntry(string_view name);
        SPtr<class MenuEntryButton> FindButtonEntry(string_view name);
    };
}