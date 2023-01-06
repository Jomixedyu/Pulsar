#pragma once
#include "EditorWindow.h"

namespace apatiteed
{
    class ConsoleWindow : public EditorWindow
    {
        CORELIB_DEF_TYPE(AssemblyObject_ApatiteEd, apatiteed::ConsoleWindow, EditorWindow);
        virtual void OnDrawImGui() override;
        virtual ImGuiWindowFlags GetGuiWindowFlags() const override;

        virtual void OnOpen() override;
        virtual void OnClose() override;
        int32_t log_selected_index = -1;
        int32_t log_level_filter = 0;
        void UnSelectLog() { this->log_selected_index = -1; }
    public:
        static string_view StaticWindowName() { return "Console"; }
        virtual string_view GetWindowName() const override { return StaticWindowName(); }
        ConsoleWindow();
    };
}