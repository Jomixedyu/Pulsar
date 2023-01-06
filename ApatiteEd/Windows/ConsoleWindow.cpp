#include <ApatiteEd/Windows/ConsoleWindow.h>
#include <Apatite/Logger.h>
#include <ApatiteEd/EditorLogRecorder.h>

namespace apatiteed
{

    void ConsoleWindow::OnDrawImGui()
    {
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::Button("Clear"))
            {
                EditorLogRecorder::Clear();
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

        auto& loglist = EditorLogRecorder::loglist;
        if (ImGui::BeginListBox("##console", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, -FLT_MIN)))
        {
            for (size_t i = 0; i < loglist.size(); i++)
            {
                bool selected = this->log_selected_index == i;
                if (ImGui::Selectable(loglist[i].record_info.c_str(), selected))
                {
                    this->log_selected_index = i;
                }

                if (selected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }

            ImGui::EndListBox();
        }
        ImGui::SameLine();
        if (this->log_selected_index >= 0)
        {
            ImGui::Text(loglist[this->log_selected_index].stacktrace_info.c_str());
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