#include "Menus/ISubMenu.h"
#include "Menus/MenuEntry.h"
#include "Menus/MenuEntrySubMenu.h"

namespace pulsared
{
    sptr<MenuEntrySubMenu> ISubMenu::FindSubMenuEntry(string_view name)
    {
        return sptr_cast<MenuEntrySubMenu>(this->FindEntry(name));
    }
    sptr<MenuEntrySubMenu> ISubMenu::FindOrNewMenuEntry(string_view name)
    {
        if (auto ret = FindSubMenuEntry(name))
        {
            return ret;
        }
        auto menu = mksptr(new MenuEntrySubMenu{ string{name} });
        AddEntry(menu);
        return menu;
    }
    sptr<MenuEntryCheck> ISubMenu::FindCheckEntry(string_view name)
    {
        return sptr_cast<MenuEntryCheck>(this->FindEntry(name));
    }
    sptr<MenuEntryButton> ISubMenu::FindButtonEntry(string_view name)
    {
        return sptr_cast<MenuEntryButton>(this->FindEntry(name));
    }
}