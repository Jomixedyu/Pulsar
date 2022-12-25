#pragma once
#include "EditorWindow.h"

namespace apatiteed
{
    class OutputWindow : public EditorWindow
    {
        virtual string_view GetWindowName() const override { return "Output"; }
        virtual void OnDrawImGui() override;
    };
}