#pragma once
#include "EditorWindow.h"

namespace apatiteed
{
    class OutputWindow : public EditorWindow
    {
        CORELIB_DEF_TYPE(AssemblyObject_ApatiteEd, apatiteed::OutputWindow, EditorWindow);
    public:
        static string_view StaticWindowName() { return "Output"; }
        virtual string_view GetWindowName() const override { return StaticWindowName(); }
        virtual void OnDrawImGui() override;
    };
}