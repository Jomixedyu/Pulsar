#pragma once
#include "EditorWindow.h"


namespace pulsared
{
    class WorkspaceWindow : public EditorWindow
    {
        CORELIB_DEF_TYPE(AssemblyObject_PulsarEd, pulsared::WorkspaceWindow, EditorWindow);
    public:
        static string_view StaticWindowName() { return ICON_FK_DATABASE " Workspace###"  "Workspace"; }
        virtual string_view GetWindowName() const override { return StaticWindowName(); }
        virtual void OnDrawImGui() override;
        WorkspaceWindow();
    protected:
        char search_buf[256];
    };
}