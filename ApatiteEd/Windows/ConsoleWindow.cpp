#include <apatiteed/Windows/ConsoleWindow.h>

namespace apatiteed
{
    void ConsoleWindow::OnDrawImGui()
    {
        if (ImGui::BeginMenuBar())
        {
            ImGui::Button("Clear");

            ImGui::Separator();

            ImGui::SetNextItemWidth(100);
            if (ImGui::BeginCombo("Log Level", "All"))
            {

                ImGui::EndCombo();
            }

            ImGui::Separator();

            static char search[255];
            ImGui::SetNextItemWidth(150);
            ImGui::InputText("Search", search, 255);

            ImGui::EndMenuBar();
        }


        ImGui::Text("[Info] initialized.");

    }
    ImGuiWindowFlags ConsoleWindow::GetGuiWindowFlags() const
    {
        return ImGuiWindowFlags_MenuBar;
    }
    ConsoleWindow::ConsoleWindow()
    {

    }
}