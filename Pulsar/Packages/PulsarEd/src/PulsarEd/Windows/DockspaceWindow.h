#pragma once

#include "EdGuiWindow.h"

namespace pulsared
{
    class DockspaceWindow : public EdGuiWindow
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::DockspaceWindow, EdGuiWindow);
    public:
        static string_view StaticWindowName() { return "Dockspace"; }
        DockspaceWindow();
        virtual string_view GetWindowDisplayName() const override { return StaticWindowName(); }
        virtual void OnDrawImGui(float dt) override;
        virtual void DrawImGui(float dt) override;
    };
}