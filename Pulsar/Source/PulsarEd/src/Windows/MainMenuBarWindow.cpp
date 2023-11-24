#include "Windows/MainMenuBarWindow.h"
#include <PulsarEd/Windows/EditorWindowManager.h>
#include <PulsarEd/Menus/Menu.h>

namespace pulsared
{
    static string FirstPath(string_view path)
    {
        auto index = path.find("/", 0);
        if (index < 0) return string{ path };
        return string{ path.substr(0, index) };
    }

    static void _ImGuiMenu(ISubMenu* menu)
    {
        for (const MenuEntry_sp& menu_item : menu->GetEntries())
        {
            if (auto submenu = interface_cast<ISubMenu>(menu_item.get()))
            {
                if (ImGui::BeginMenu(menu_item->Name.data(), true))
                {
                    _ImGuiMenu(submenu);
                    ImGui::EndMenu();
                }
            }
            else if(sptr<MenuEntryButton> btn = sptr_cast<MenuEntryButton>(menu_item))
            {
                ImGui::PushID(btn->Name.c_str());
                if (ImGui::MenuItem(btn->DisplayName.c_str(), nullptr))
                {
                    if (btn->Action)
                    {
                        btn->Action->Invoke(nullptr);
                    }
                }
                ImGui::PopID();
            }
            else if (sptr<MenuEntryCheck> check = sptr_cast<MenuEntryCheck>(menu_item))
            {
                ImGui::PushID(check->Name.c_str());
                if (ImGui::MenuItem(check->DisplayName.c_str(), nullptr, check->IsChecked))
                {
                    if (check->CheckedAction)
                    {
                        check->IsChecked = !check->IsChecked;
                        auto ctx = MenuContexts::StaticMakeContext(check->Name, nullptr);
                        check->CheckedAction->Invoke(ctx, check->IsChecked);
                    }
                }
                ImGui::PopID();
            }
           
        }
    }

    void MainMenuBarWindow::OnDrawImGui()
    {
        if (ImGui::BeginMainMenuBar())
        {
            _ImGuiMenu(MenuManager::GetMenu("Main").get());
            ImGui::EndMainMenuBar();
        }
    }
    MainMenuBarWindow::MainMenuBarWindow()
    {

    }
}