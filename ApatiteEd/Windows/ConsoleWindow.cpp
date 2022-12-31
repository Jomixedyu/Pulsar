#include <ApatiteEd/Windows/ConsoleWindow.h>
#include <Apatite/Logger.h>

namespace apatiteed
{
    static array_list<string> log_recorder;

    void ConsoleWindow::OnDrawImGui()
    {
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::Button("Clear"))
            {
                log_recorder.clear();
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


        
        for (auto& rec : log_recorder)
        {
            ImGui::Text(rec.c_str());
        }
    }
    ImGuiWindowFlags ConsoleWindow::GetGuiWindowFlags() const
    {
        return ImGuiWindowFlags_MenuBar;
    }
    static void _LoggerListener(LogLevel level, string_view log)
    {
        string info;
        switch (level)
        {
        case apatite::LogLevel::Info:
            info = "[Info]";
            break;
        case apatite::LogLevel::Warning:
            info = "[Warning]";
            break;
        case apatite::LogLevel::Error:
            info = "[Error]";
            break;
        default:
            break;
        }
        log_recorder.push_back(info + string{ log });
    }
    void ConsoleWindow::OnOpen()
    {
        Logger::LogListener += _LoggerListener;
    }

    void ConsoleWindow::OnClose()
    {
        Logger::LogListener -= _LoggerListener;
    }

    ConsoleWindow::ConsoleWindow()
    {

    }
}