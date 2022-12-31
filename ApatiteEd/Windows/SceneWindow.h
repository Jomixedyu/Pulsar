#pragma once
#include "EditorWindow.h"

namespace apatiteed
{
    class SceneWindow : public EditorWindow
    {
        CORELIB_DEF_TYPE(AssemblyObject_ApatiteEd, apatiteed::SceneWindow, EditorWindow);
    public:
        SceneWindow();
        virtual void OnOpen() override;
        virtual void OnClose() override;
        static string_view StaticWindowName() { return "Scene"; }
        virtual string_view GetWindowName() const override { return StaticWindowName(); }
        virtual void OnDrawImGui() override;
    };
}