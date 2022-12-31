#pragma once
#include "EditorWindow.h"

namespace apatiteed
{
    class ProjectWindow : public EditorWindow
    {
        CORELIB_DEF_TYPE(AssemblyObject_ApatiteEd, apatiteed::ProjectWindow, EditorWindow);
    public:
        static string_view StaticWindowName() { return "Project"; }
        virtual string_view GetWindowName() const override { return StaticWindowName(); }
        virtual void OnDrawImGui() override;
    };
}