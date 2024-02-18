#pragma once

#include "EditorWindow.h"

namespace pulsared
{
    class DockspaceWindow : public EditorWindow
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::DockspaceWindow, EditorWindow);
    public:
        static string_view StaticWindowName() { return "Dockspace"; }
        DockspaceWindow();
        virtual string_view GetWindowDisplayName() const override { return StaticWindowName(); }
        virtual void OnDrawImGui(float dt) override;
        virtual void DrawImGui(float dt) override { this->OnDrawImGui(dt); }
    };
}