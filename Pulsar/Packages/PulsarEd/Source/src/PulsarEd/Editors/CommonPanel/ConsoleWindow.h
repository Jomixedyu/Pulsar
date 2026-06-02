#pragma once
#include "Windows/PanelWindow.h"

namespace pulsared
{
    class ConsoleWindow : public PanelWindow
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::ConsoleWindow, PanelWindow);
    protected:
        virtual void OnDrawImGui(float dt) override;
        virtual ImGuiWindowFlags GetGuiWindowFlags() const override;

        virtual void OnOpen() override;
        virtual void OnClose() override;

        void UnSelectLog() { this->m_logSelectedIndex = -1; }
    public:
        static string_view StaticWindowName() { return ICON_FK_WINDOW_MAXIMIZE " Console###" "Console"; }
        virtual string_view GetWindowDisplayName() const override { return StaticWindowName(); }

        ConsoleWindow();
    protected:
        int32_t m_logSelectedIndex = -1;
        int32_t m_logLevelFilter = 0;
    };
}