#pragma once

#include "EditorWindow.h"

namespace apatiteed
{
    class DockspaceWindow : public EditorWindow
    {
    public:
        virtual string_view GetWindowName() const override { return "Dockspace"; }
        virtual void OnDrawImGui() override;

        virtual void DrawImGui() override { this->OnDrawImGui(); }
    };
}