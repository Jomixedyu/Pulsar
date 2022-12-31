#pragma once
#include "EditorWindow.h"

namespace apatiteed
{
    class ConsoleWindow : public EditorWindow
    {
        CORELIB_DEF_TYPE(AssemblyObject_ApatiteEd, apatiteed::ConsoleWindow, EditorWindow);
        virtual void OnDrawImGui() override;
        virtual ImGuiWindowFlags GetGuiWindowFlags() const override;
    public:
        static string_view StaticWindowName() { return "Console"; }
        virtual string_view GetWindowName() const override { return StaticWindowName(); }
        ConsoleWindow();
    };
}