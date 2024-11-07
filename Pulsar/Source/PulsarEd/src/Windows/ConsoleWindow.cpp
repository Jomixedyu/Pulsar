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
            if (ImGui::BeginCombo("Log Level", log_levels[this->m_logLevelFilter]))
            {
                for (int32_t i = 0; i < 4; i++)
                {
                    bool selected = this->m_logLevelFilter == i;
                    if (ImGui::Selectable(log_levels[i], selected))
                    {
                        this->m_logLevelFilter = i;
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
        if (this->m_logSelectedIndex >= 0)
        {
            ImGui::Text(loglist[this->m_logSelectedIndex].stacktrace_info.c_str());
        }
        ImGui::EndChild();
        ImGui::Separator();

        if (ImGui::BeginListBox("##console", ImVec2(-FLT_MIN, -FLT_MIN)))
        {
            for (int32_t i = (int32_t)loglist.size() - 1; i >= 0; --i)
            {
                constexpr int kAllLevel = 0;
                if (this->m_logLevelFilter != kAllLevel && this->m_logLevelFilter != (int32_t)loglist[i].level)
                {
                    continue;
                }
                bool selected = this->m_logSelectedIndex == i;

                bool modify_color = false;
                if (loglist[i].level == LogLevel::Info)
                {
                    ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_Text, ImVec4{ 0.9f, 0.9f,0.9f,1.f });
                    modify_color = true;
                }
                else if (loglist[i].level == LogLevel::Warning)
                {
                    ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_Text, ImVec4{ 1.f, 1.f, 0.3f, 1.f });
                    modify_color = true;
                }
                else if (loglist[i].level == LogLevel::Error)
                {
                    ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_Text, ImVec4{ 1.f,0.2f,0.2f,1.f });
                    modify_color = true;
                }

                bool grid = i % 2 == 0 && !selected;

                if (grid)
                {

                }

                ImGui::PushID(i);
                if (ImGui::Selectable(loglist[i].record_info.c_str(), selected))
                {
                    this->m_logSelectedIndex = i;
                }
                ImGui::PopID();

                if (modify_color)
                {
                    ImGui::PopStyleColor();
                }

                if (grid)
                {

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