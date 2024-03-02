#pragma once
#include <PulsarEd/Assembly.h>
#include <PulsarEd/Windows/ToolWindow.h>

namespace pulsared
{

    class MenuDebugTool : public ToolWindow
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::MenuDebugTool, ToolWindow);
    public:
        virtual string_view GetWindowDisplayName() const override { return ICON_FK_WRENCH " MenuDebugTool"; }
        virtual void OnDrawImGui(float dt);
    };
}