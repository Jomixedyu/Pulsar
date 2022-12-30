#pragma once
#include "EditorWindow.h"

namespace apatiteed
{
    class MainMenuBarWindow : public EditorWindow
    {
    protected:
        virtual void OnDrawImGui() override;
    public:
        virtual string_view GetWindowName() const override { return "MainMenuBar"; }
        virtual void DrawImGui() override { this->OnDrawImGui(); }
        virtual bool get_is_register_menu() const override { return false; }
        MainMenuBarWindow();
    };
}