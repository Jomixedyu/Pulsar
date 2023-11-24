#include <PulsarEd/Windows/PanelWindow.h>
#include <PulsarEd/Menus/Menu.h>
#include <PulsarEd/Menus/MenuEntrySubMenu.h>

namespace pulsared
{

    void PanelWindow::OnOpen()
    {
        base::OnOpen();
        Logger::Log("open panel: " + GetType()->GetName());
        GetCheckedEntry()->IsChecked = true;
    }
    void PanelWindow::OnClose()
    {
        base::OnClose();
        Logger::Log("close panel: " + GetType()->GetName());
        GetCheckedEntry()->IsChecked = false;
    }

    MenuEntryCheck_sp PanelWindow::GetCheckedEntry() const
    {
        auto menu = MenuManager::GetMainMenu()->FindSubMenuEntry("Window");
        return menu->FindCheckEntry(GetWindowName());
    }
}