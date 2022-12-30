#include "MainMenuBarWindow.h"
#include <ApatiteEd/Windows/EditorWindowManager.h>
#include <ApatiteEd/Menus/Menu.h>

namespace apatiteed
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
                if (ImGui::BeginMenu(menu_item->name.data(), true))
                {
                    _ImGuiMenu(submenu);
                    ImGui::EndMenu();
                }
            }
            else if(sptr<MenuEntryButton> btn = sptr_cast<MenuEntryButton>(menu_item))
            {
                if (ImGui::Button(btn->name.c_str()))
                {
                    if (btn->action)
                    {
                        btn->action->Invoke(nullptr);
                    }
                }
            }
            else if (sptr<MenuEntryCheck> check = sptr_cast<MenuEntryCheck>(menu_item))
            {
                if (ImGui::MenuItem(check->name.c_str(), nullptr, check->is_check))
                {
                    if (check->check_action)
                    {
                        check->is_check = !check->is_check;
                        auto ctx = MenuContexts::StaticMakeContext(check->name, nullptr);
                        check->check_action->Invoke(ctx, check->is_check);
                    }
                }
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