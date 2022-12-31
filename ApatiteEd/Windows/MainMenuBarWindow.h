#pragma once
#include "EditorWindow.h"

namespace apatiteed
{
    class MainMenuBarWindow : public EditorWindow
    {
        CORELIB_DEF_TYPE(AssemblyObject_ApatiteEd, apatiteed::MainMenuBarWindow, EditorWindow);
    protected:
        virtual void OnDrawImGui() override;
    public:
        static string_view StaticWindowName() { return "MainMenuBar"; }
        virtual string_view GetWindowName() const override { return StaticWindowName(); }
        virtual void DrawImGui() override { this->OnDrawImGui(); }
        MainMenuBarWindow();
    };
}