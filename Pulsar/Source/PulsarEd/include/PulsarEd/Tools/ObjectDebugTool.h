#pragma once
#include <PulsarEd/Assembly.h>
#include <PulsarEd/Windows/ToolWindow.h>

namespace pulsared
{

    class ObjectDebugTool : public ToolWindow
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::ObjectDebugTool, ToolWindow);
    public:
        ObjectDebugTool();
        virtual string_view GetWindowDisplayName() const override { return ICON_FK_WRENCH " ObjectDebugTool"; }
        virtual void OnDrawImGui(float dt);

    private:
        char m_search[128]{};
    };
}