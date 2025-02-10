#pragma once
#include "Windows/PanelWindow.h"
#include <Pulsar/Node.h>

namespace pulsared
{
    class OutlinerWindow : public PanelWindow
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::OutlinerWindow, PanelWindow);
    public:
        static string_view StaticWindowName() { return ICON_FK_TREE " Outliner###" "Outerliner"; }
        virtual string_view GetWindowDisplayName() const override { return StaticWindowName(); }
        virtual void OnDrawImGui(float dt) override;
    public:
    };
}