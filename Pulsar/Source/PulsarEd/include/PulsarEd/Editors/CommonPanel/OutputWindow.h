#pragma once
#include "Windows/PanelWindow.h"

namespace pulsared
{
    class OutputWindow : public PanelWindow
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::OutputWindow, PanelWindow);
    public:
        static string_view StaticWindowName() { return ICON_FK_TELEVISION " Output" "###Output"; }
        virtual string_view GetWindowDisplayName() const override { return StaticWindowName(); }
        virtual void OnDrawImGui(float dt) override;
    };
}