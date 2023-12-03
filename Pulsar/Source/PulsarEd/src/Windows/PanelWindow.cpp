#include <PulsarEd/Menus/Menu.h>
#include <PulsarEd/Menus/MenuEntrySubMenu.h>
#include <PulsarEd/Windows/PanelWindow.h>

namespace pulsared
{

    void PanelWindow::OnOpen()
    {
        base::OnOpen();
        Logger::Log("open panel: " + GetType()->GetName());

        if (const auto entry = GetCheckedEntry())
            entry->IsChecked = true;
    }
    void PanelWindow::OnClose()
    {
        base::OnClose();
        Logger::Log("close panel: " + GetType()->GetName());

        if (const auto entry = GetCheckedEntry())
            entry->IsChecked = false;
    }

    MenuEntryCheck_sp PanelWindow::GetCheckedEntry() const
    {
        auto menu = MenuManager::GetMainMenu()->FindSubMenuEntry("Window");
        return menu->FindCheckEntry(GetWindowName());
    }
} // namespace pulsared