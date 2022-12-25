#pragma once
#include "EditorWindow.h"

namespace apatiteed
{
    class SceneWindow : public EditorWindow
    {
    public:
        SceneWindow();
        virtual void OnOpen() override;
        virtual void OnClose() override;
        virtual string_view GetWindowName() const override { return "Scene"; }
        virtual void OnDrawImGui() override;
    };
}