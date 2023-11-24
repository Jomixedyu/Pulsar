#pragma once
#include "PanelWindow.h"

namespace pulsared
{
    class ConsoleWindow : public PanelWindow
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::ConsoleWindow, PanelWindow);
        virtual void OnDrawImGui() override;
        virtual ImGuiWindowFlags GetGuiWindowFlags() const override;

        virtual void OnOpen() override;
        virtual void OnClose() override;
        int32_t log_selected_index = -1;
        int32_t log_level_filter = 0;
        void UnSelectLog() { this->log_selected_index = -1; }
    public:
        static string_view StaticWindowName() { return ICON_FK_WINDOW_MAXIMIZE " Console###" "Console"; }
        virtual string_view GetWindowDisplayName() const override { return StaticWindowName(); }

        ConsoleWindow();
    };
}