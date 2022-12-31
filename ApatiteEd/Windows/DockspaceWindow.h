#pragma once

#include "EditorWindow.h"

namespace apatiteed
{
    class DockspaceWindow : public EditorWindow
    {
        CORELIB_DEF_TYPE(AssemblyObject_ApatiteEd, apatiteed::DockspaceWindow, EditorWindow);
    public:
        static string_view StaticWindowName() { return "Dockspace"; }
        virtual string_view GetWindowName() const override { return StaticWindowName(); }
        virtual void OnDrawImGui() override;
        virtual void DrawImGui() override { this->OnDrawImGui(); }
    };
}