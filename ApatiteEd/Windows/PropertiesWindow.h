#pragma once
#include "EditorWindow.h"

namespace apatiteed
{
    class PropertiesWindow : public EditorWindow
    {
        virtual string_view GetWindowName() const override { return "Properties"; }
        virtual void OnDrawImGui() override;

    };
}