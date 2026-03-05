#include "Menus/ISubMenu.h"
#include "Menus/MenuEntry.h"
#include "Menus/MenuEntrySubMenu.h"

namespace pulsared
{
    SPtr<MenuEntrySubMenu> ISubMenu::FindSubMenuEntry(string_view name)
    {
        return sptr_cast<MenuEntrySubMenu>(this->FindEntry(name));
    }
    SPtr<MenuEntrySubMenu> ISubMenu::FindOrNewMenuEntry(string_view name)
    {
        if (auto ret = FindSubMenuEntry(name))
        {
            return ret;
        }
        auto menu = mksptr(new MenuEntrySubMenu{ string{name} });
        AddEntry(menu);
        return menu;
    }
    SPtr<MenuEntryCheck> ISubMenu::FindCheckEntry(string_view name)
    {
        return sptr_cast<MenuEntryCheck>(this->FindEntry(name));
    }
    SPtr<MenuEntryButton> ISubMenu::FindButtonEntry(string_view name)
    {
        return sptr_cast<MenuEntryButton>(this->FindEntry(name));
    }
}