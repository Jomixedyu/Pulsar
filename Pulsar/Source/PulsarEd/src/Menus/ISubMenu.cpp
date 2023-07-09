#include "ISubMenu.h"
#include "MenuEntry.h"
#include "MenuEntrySubMenu.h"

namespace pulsared
{
    sptr<MenuEntrySubMenu> ISubMenu::FindSubMenuEntry(string_view name)
    {
        return sptr_cast<MenuEntrySubMenu>(this->FindEntry(name));
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