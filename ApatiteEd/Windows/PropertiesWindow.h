#pragma once
#include "EditorWindow.h"

namespace apatiteed
{
    class PropertiesWindow : public EditorWindow
    {
        CORELIB_DEF_TYPE(AssemblyObject_ApatiteEd, apatiteed::PropertiesWindow, EditorWindow);
    public:
        static string_view StaticWindowName() { return "Properties"; }
        virtual string_view GetWindowName() const override { return StaticWindowName(); }
        virtual void OnDrawImGui() override;

    };
}