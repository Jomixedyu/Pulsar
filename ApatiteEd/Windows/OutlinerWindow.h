#pragma once
#include "EditorWindow.h"

namespace apatiteed
{
    class OutlinerWindow : public EditorWindow
    {
        virtual string_view GetWindowName() const override { return "Outliner"; }
        virtual void OnDrawImGui() override;
    };
}