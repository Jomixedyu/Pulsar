#pragma once
#include <PulsarEd/Assembly.h>
#include <PulsarEd/Windows/ToolWindow.h>

namespace pulsared
{

    class StaticMeshAttributeTool : public ToolWindow
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::StaticMeshAttributeTool, ToolWindow);
    public:
        StaticMeshAttributeTool();
        virtual string_view GetWindowDisplayName() const override { return ICON_FK_WRENCH " StaticMeshAttributeTool"; }
        virtual void OnDrawImGui(float dt);

    };
}