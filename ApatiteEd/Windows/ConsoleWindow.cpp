#include <ApatiteEd/Windows/ConsoleWindow.h>
#include <Apatite/Logger.h>
#include <ApatiteEd/LogRecorder.h>

namespace apatiteed
{

    void ConsoleWindow::OnDrawImGui()
    {
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::Button("Clear"))
            {
                LogRecorder::Clear();
            }

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


        for (auto& rec : LogRecorder::loglist)
        {
            ImGui::Text(rec.c_str());
        }
    }
    ImGuiWindowFlags ConsoleWindow::GetGuiWindowFlags() const
    {
        return ImGuiWindowFlags_MenuBar;
    }

    void ConsoleWindow::OnOpen()
    {

    }

    void ConsoleWindow::OnClose()
    {

    }

    ConsoleWindow::ConsoleWindow()
    {

    }
}