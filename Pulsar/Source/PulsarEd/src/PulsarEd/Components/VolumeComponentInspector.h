#pragma once
#include <Pulsar/Components/VolumeComponent.h>
#include <PulsarEd/PropertyControls/ComponentInspector.h>

namespace pulsared
{
    class VolumeComponentInspector : public ComponentInspector
    {
        EDITOR_IMPL_COMPONENT_INSPECTOR(pulsar::VolumeComponent, new VolumeComponentInspector);
    public:
        void OnDrawImGui(pulsar::Component* comp, bool showDebug) override;
    };
}
