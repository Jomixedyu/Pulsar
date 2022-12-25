#pragma once
#include "EditorWindow.h"

namespace apatiteed
{
    class ConsoleWindow : public EditorWindow
    {
        virtual string_view GetWindowName() const override { return "Console"; }
        virtual void OnDrawImGui() override;
        virtual ImGuiWindowFlags GetGuiWindowFlags() const override;
    public:
        ConsoleWindow();
    };
}