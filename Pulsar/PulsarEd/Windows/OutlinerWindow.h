#pragma once
#include "EditorWindow.h"
#include <Pulsar/Node.h>

namespace pulsared
{
    class OutlinerWindow : public EditorWindow
    {
        CORELIB_DEF_TYPE(AssemblyObject_PulsarEd, pulsared::OutlinerWindow, EditorWindow);
    public:
        static string_view StaticWindowName() { return "Outliner"; }
        virtual string_view GetWindowName() const override { return StaticWindowName(); }
        virtual void OnDrawImGui() override;

    public:
    };
}