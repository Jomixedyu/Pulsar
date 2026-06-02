#include <PulsarEd/Menus/Menu.h>
#include <PulsarEd/Menus/MenuEntrySubMenu.h>
#include <PulsarEd/Windows/PanelWindow.h>
#include <imgui/imgui_internal.h>

namespace pulsared
{
    MenuEntryCheck_sp PanelWindow::GetCheckedEntry() const
    {
        auto menu = MenuManager::GetMainMenu()->FindSubMenuEntry("Window");
        return menu->FindCheckEntry(GetWindowName());
    }

    ImGuiWindowClass PanelWindow::GetGuiWindowClass() const
    {
        ImGuiWindowClass winClass{};
        winClass.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoWindowMenuButton;
        return winClass;
    }
}
