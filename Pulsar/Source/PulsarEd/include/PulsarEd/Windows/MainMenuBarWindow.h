#pragma once
#include "EditorWindow.h"

namespace pulsared
{
    class MainMenuBarWindow : public EditorWindow
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::MainMenuBarWindow, EditorWindow);
    protected:
        virtual void OnDrawImGui(float dt) override;
    public:
        static string_view StaticWindowName() { return "MainMenuBar"; }
        virtual string_view GetWindowDisplayName() const override { return StaticWindowName(); }
        virtual void DrawImGui(float dt) override { this->OnDrawImGui(dt); }
        MainMenuBarWindow();
    };
}