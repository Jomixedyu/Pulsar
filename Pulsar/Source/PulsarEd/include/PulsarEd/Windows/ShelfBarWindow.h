#pragma once
#include "PanelWindow.h"

namespace pulsared
{
    class ShelfBarWindow : public EdGuiWindow
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::ShelfBarWindow, EdGuiWindow);
    public:
        virtual ImGuiWindowFlags GetGuiWindowFlags() const override;
        static string_view StaticWindowName() { return ICON_FK_TELEVISION " ShelfBar" "###ShelfBar"; }
        virtual string_view GetWindowDisplayName() const override { return StaticWindowName(); }
        virtual void OnDrawImGui(float dt) override;
        ShelfBarWindow();
    };
}