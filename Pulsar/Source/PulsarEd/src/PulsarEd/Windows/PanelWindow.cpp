#include <PulsarEd/Menus/Menu.h>
#include <PulsarEd/Menus/MenuEntrySubMenu.h>
#include <PulsarEd/Windows/PanelWindow.h>

namespace pulsared
{
    MenuEntryCheck_sp PanelWindow::GetCheckedEntry() const
    {
        auto menu = MenuManager::GetMainMenu()->FindSubMenuEntry("Window");
        return menu->FindCheckEntry(GetWindowName());
    }
}
