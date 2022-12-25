#include "MainMenuBarWindow.h"
#include <ApatiteEd/Windows/EditorWindowManager.h>

namespace apatiteed
{
    void MainMenuBarWindow::OnDrawImGui()
    {
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Edit"))
            {
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Build"))
            {
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Window"))
            {
                for (auto& window : EditorWindowManager::GetInstance()->GetWindows())
                {
                    bool opened = window->get_is_opened();
                    ImGui::MenuItem(window->GetWindowName().data(), nullptr, &opened, true);
                    
                }

                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Help"))
            {
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
    }
}