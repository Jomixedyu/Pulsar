#pragma once

#include "EditorWindow.h"

namespace apatiteed
{
    class DockspaceWindow : public EditorWindow
    {
    public:
        virtual string_view GetWindowName() const override { return "Dockspace"; }
        virtual void OnDrawImGui() override;
        virtual bool get_is_register_menu() const override { return false; }
        virtual void DrawImGui() override { this->OnDrawImGui(); }
    };
}