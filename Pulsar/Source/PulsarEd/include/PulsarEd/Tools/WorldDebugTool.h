#pragma once
#include <PulsarEd/Assembly.h>
#include <PulsarEd/Windows/ToolWindow.h>

namespace pulsared
{

    class WorldDebugTool : public ToolWindow
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::WorldDebugTool, ToolWindow);
    public:
        WorldDebugTool();
        virtual string_view GetWindowDisplayName() const override { return ICON_FK_WRENCH " WorldDebugTool"; }
        virtual void OnDrawImGui(float dt);

    };
}