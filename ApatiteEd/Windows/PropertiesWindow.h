#pragma once
#include "EditorWindow.h"

#define APATITEED_PROPERTIES_PANEL(TARGET_TYPE) \
static inline struct __properties_panel{ \
__properties_panel(){ \
    PropertiesWindow::StaticRegisterPropertiesPanel(TARGET_TYPE, StaticType()); } \
} __properties_panel_;

namespace apatiteed
{
    class PropertiesPanel : public Object
    {
        CORELIB_DEF_TYPE(AssemblyObject_ApatiteEd, apatiteed::PropertiesPanel, Object);

    public:
        virtual void OnDrawImGui() = 0;
    };

    CORELIB_DECL_SHORTSPTR(PropertiesPanel);

    class PropertiesWindow : public EditorWindow
    {
        CORELIB_DEF_TYPE(AssemblyObject_ApatiteEd, apatiteed::PropertiesWindow, EditorWindow);
    public:
        static string_view StaticWindowName() { return "Properties"; }
        virtual string_view GetWindowName() const override { return StaticWindowName(); }
        virtual void OnDrawImGui() override;
        virtual void OnOpen() override;
        virtual void OnClose() override;

    public:
        static void StaticRegisterPropertiesPanel(Type* target_type, Type* panel_type);
    protected:
        PropertiesPanel_sp current_panel;
    };
    CORELIB_DECL_SHORTSPTR(PropertiesWindow);
}