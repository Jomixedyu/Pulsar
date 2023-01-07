#pragma once
#include <ApatiteEd/Windows/PropertiesWindow.h>
#include <Apatite/Node.h>

namespace apatiteed
{

    class PropertiesNodePanel : public PropertiesPanel
    {
        CORELIB_DEF_TYPE(AssemblyObject_Apatite, apatiteed::PropertiesPanel, Object);
        APATITEED_PROPERTIES_PANEL(cltypeof<Node>());
    public:

        virtual void OnDrawImGui() override;
    };
}