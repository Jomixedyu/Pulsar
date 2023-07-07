#pragma once
#include "EditorWindow.h"

namespace pulsared
{
    class OutputWindow : public EditorWindow
    {
        CORELIB_DEF_TYPE(AssemblyObject_PulsarEd, pulsared::OutputWindow, EditorWindow);
    public:
        static string_view StaticWindowName() { return "Output"; }
        virtual string_view GetWindowName() const override { return StaticWindowName(); }
        virtual void OnDrawImGui() override;
    };
}