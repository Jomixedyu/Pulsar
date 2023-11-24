#pragma once
#include <PulsarEd/Windows/PropertiesWindow.h>
#include <Pulsar/Node.h>

namespace pulsared
{

    class PropertiesNodePanel : public PropertiesPanel
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::PropertiesNodePanel, PropertiesPanel);
        APATITEED_PROPERTIES_PANEL(cltypeof<Node>());

    public:

        virtual void OnDrawImGui() override;
    };
}