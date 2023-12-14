#include <PulsarEd/Windows/ConsoleWindow.h>
#include <Pulsar/Logger.h>
#include <PulsarEd/EditorLogRecorder.h>

namespace pulsared
{

    void ConsoleWindow::OnDrawImGui(float dt)
    {
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::Button("Clear"))
            {
                EditorLogRecorder::Clear();
                this->UnSelectLog();
            }

            ImGui::Separator();

            ImGui::SetNextItemWidth(100);

            static const char* log_levels[] = { "All", "Info", "Warning", "Error" };
            if (ImGui::BeginCombo("Log Level", log_levels[this->log_level_filter]))
            {
                for (size_t i = 0; i < 4; i++)
                {
                    bool selected = this->log_level_filter == i;
                    if (ImGui::Selectable(log_levels[i], selected))
                    {
                        this->log_level_filter = i;
                        this->UnSelectLog();
                    }
                }
                ImGui::EndCombo();
            }

            ImGui::Separator();

            static char search[255];
            ImGui::SetNextItemWidth(150);
            ImGui::InputText("Search", search, 255);

            ImGui::EndMenuBar();
        }

        auto& loglist = EditorLogRecorder::loglist;


        ImGui::BeginChild("##console detail", ImVec2{ -FLT_MIN, ImGui::GetContentRegionAvail().y * 0.5f }, false, ImGuiWindowFlags_::ImGuiWindowFlags_HorizontalScrollbar);
        if (this->log_selected_index >= 0)
        {
            ImGui::Text(loglist[this->log_selected_index].stacktrace_info.c_str());
        }
        ImGui::EndChild();
        ImGui::Separator();

        if (ImGui::BeginListBox("##console", ImVec2(-FLT_MIN, -FLT_MIN)))
        {
            for (int32_t i = loglist.size() - 1; i >= 0; --i)
            {
                constexpr int kAllLevel = 0;
                if (this->log_level_filter != kAllLevel && this->log_level_filter != (int32_t)loglist[i].level)
                {
                    continue;
                }
                bool selected = this->log_selected_index == i;

                bool modify_color = false;
                if (loglist[i].level == LogLevel::Info)
                {
                    ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_Text, ImVec4{ 0.9,0.9,0.9,1 });
                    modify_color = true;
                }
                else if (loglist[i].level == LogLevel::Warning)
                {
                    ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_Text, ImVec4{ 1,1,0.3,1 });
                    modify_color = true;
                }
                else if (loglist[i].level == LogLevel::Error)
                {
                    ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_Text, ImVec4{ 1,0.2,0.2,1 });
                    modify_color = true;
                }

                ImGui::PushID(i);
                if (ImGui::Selectable(loglist[i].record_info.c_str(), selected))
                {
                    this->log_selected_index = i;
                }
                ImGui::PopID();

                if (modify_color)
                {
                    ImGui::PopStyleColor();
                }

                if (selected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }

            ImGui::EndListBox();
        }


    }
    ImGuiWindowFlags ConsoleWindow::GetGuiWindowFlags() const
    {
        return ImGuiWindowFlags_MenuBar;
    }

    void ConsoleWindow::OnOpen()
    {
        base::OnOpen();
    }

    void ConsoleWindow::OnClose()
    {
        base::OnClose();
    }

    ConsoleWindow::ConsoleWindow()
    {

    }
}