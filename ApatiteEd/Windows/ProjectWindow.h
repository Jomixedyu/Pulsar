#pragma once
#include "EditorWindow.h"

namespace apatiteed
{
    class ProjectWindow : public EditorWindow
    {
        virtual string_view GetWindowName() const override { return "Project"; }
        virtual void OnDrawImGui() override;
    };
}