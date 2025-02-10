#pragma once
#include "Windows/PanelWindow.h"

#define APATITEED_PROPERTIES_PANEL(TARGET_TYPE) \
static inline struct __properties_panel{ \
__properties_panel(){ \
    PropertiesWindow::StaticRegisterPropertiesPanel(TARGET_TYPE, StaticType()); } \
} __properties_panel_;

namespace pulsared
{
    class PropertiesPanel : public Object
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::PropertiesPanel, Object);

    public:
        virtual void OnDrawImGui() = 0;
    };

    CORELIB_DECL_SHORTSPTR(PropertiesPanel);

    class PropertiesWindow : public PanelWindow
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::PropertiesWindow, PanelWindow);
    public:
        virtual ImGuiWindowFlags GetGuiWindowFlags() const override{
            return ImGuiWindowFlags_AlwaysVerticalScrollbar;
        }
        static string_view StaticWindowName() { return ICON_FK_TH_LIST " Properties###" "Properties"; }
        virtual string_view GetWindowDisplayName() const override { return StaticWindowName(); }
        virtual void OnDrawImGui(float dt) override;
        virtual void OnOpen() override;
        virtual void OnClose() override;

    public:
        static void StaticRegisterPropertiesPanel(Type* target_type, Type* panel_type);
    protected:
        PropertiesPanel_sp current_panel;
    };
    CORELIB_DECL_SHORTSPTR(PropertiesWindow);
}