#pragma once
#include "EditorWindow.h"
#include <Apatite/Node.h>

namespace apatiteed
{
    class OutlinerWindow : public EditorWindow
    {
        CORELIB_DEF_TYPE(AssemblyObject_ApatiteEd, apatiteed::OutlinerWindow, EditorWindow);
    public:
        static string_view StaticWindowName() { return "Outliner"; }
        virtual string_view GetWindowName() const override { return StaticWindowName(); }
        virtual void OnDrawImGui() override;

    public:
    };
}