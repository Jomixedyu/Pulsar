#include "Components/VolumeComponentInspector.h"
#include <Pulsar/Components/VolumeComponent.h>
#include <Pulsar/Assets/VolumeProfile.h>
#include <PulsarEd/PropertyControls/PropertyControl.h>
#include <PulsarEd/Editors/VolumeProfileEditor/VolumeProfileEditorWindow.h>

namespace pulsared
{
    void VolumeComponentInspector::OnDrawImGui(pulsar::Component* comp, bool showDebug)
    {
        ComponentInspector::OnDrawImGui(comp, showDebug);

        auto volComp = dynamic_cast<pulsar::VolumeComponent*>(comp);
        if (!volComp)
            return;

        auto profile = volComp->GetProfile();
        if (!profile)
            return;

        VolumeProfileEditorWindow::DrawEffectsList(profile);
    }
}
