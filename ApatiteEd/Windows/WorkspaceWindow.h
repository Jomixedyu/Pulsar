#pragma once
#include "EditorWindow.h"

namespace apatiteed
{
    class WorkspaceWindow : public EditorWindow
    {
        CORELIB_DEF_TYPE(AssemblyObject_ApatiteEd, apatiteed::WorkspaceWindow, EditorWindow);
    public:
        static string_view StaticWindowName() { return "Workspace"; }
        virtual string_view GetWindowName() const override { return StaticWindowName(); }
        virtual void OnDrawImGui() override;
        WorkspaceWindow();
    protected:
        char search_buf[256];
    };
}