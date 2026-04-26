#include <PulsarEd/Menus/Menu.h>
#include <PulsarEd/Menus/MenuEntrySubMenu.h>
#include <PulsarEd/Windows/PanelWindow.h>

namespace pulsared
{

    void PanelWindow::OnOpen()
    {
        base::OnOpen();
        Logger::Log("open panel: " + GetType()->GetName());

    }
    void PanelWindow::OnClose()
    {
        base::OnClose();
        Logger::Log("close panel: " + GetType()->GetName());
    }

    ImGuiWindowClass PanelWindow::GetGuiWindowClass() const
    {
        ImGuiWindowClass winClass{};
        winClass.DockingAllowUnclassed = false;
        auto name = StringUtil::Concat(to_string(GetParentWindowId()), "+panel");
        winClass.ClassId = ImGui::GetID(name.c_str());
        return winClass;
    }

    MenuEntryCheck_sp PanelWindow::GetCheckedEntry() const
    {
        auto menu = MenuManager::GetMainMenu()->FindSubMenuEntry("Window");
        return menu->FindCheckEntry(GetWindowName());
    }
} // namespace pulsared