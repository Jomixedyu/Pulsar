#pragma once

#include "EditorWindow.h"

namespace pulsared
{
    class DockspaceWindow : public EditorWindow
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::DockspaceWindow, EditorWindow);
    public:
        static string_view StaticWindowName() { return "Dockspace"; }
        virtual string_view GetWindowDisplayName() const override { return StaticWindowName(); }
        virtual void OnDrawImGui() override;
        virtual void DrawImGui() override { this->OnDrawImGui(); }
    };
}