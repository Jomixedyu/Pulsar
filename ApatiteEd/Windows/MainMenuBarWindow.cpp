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
    void MainMenuBarWindow::OnDrawImGui()
    {
        if (ImGui::BeginMainMenuBar())
        {
            for (auto& menu_item : MenuManager::GetMenu("Main")->GetEntries())
            {
                if (ImGui::BeginMenu(menu_item->name.data(), true))
                {
                    ImGui::EndMenu();
                }
            }
            //if (ImGui::BeginMenu("File"))
            //{
            //    ImGui::EndMenu();
            //}
            //if (ImGui::BeginMenu("Edit"))
            //{
            //    ImGui::EndMenu();
            //}
            //if (ImGui::BeginMenu("Build"))
            //{
            //    ImGui::EndMenu();
            //}
            //if (ImGui::BeginMenu("Window"))
            //{
            //    for (auto& window : EditorWindowManager::GetInstance()->GetWindows())
            //    {
            //        bool opened = window->get_is_opened();
            //        ImGui::MenuItem(window->GetWindowName().data(), nullptr, &opened, true);

            //    }

            //    ImGui::EndMenu();
            //}
            //if (ImGui::BeginMenu("Help"))
            //{
            //    ImGui::EndMenu();
            //}
            ImGui::EndMainMenuBar();
        }
    }
}