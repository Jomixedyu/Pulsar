#pragma once
#include "LightComponent.h"
#include "Pulsar/Rendering/RenderObject.h"

namespace pulsar
{
    class DirectionalLightComponent : public LightComponent
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::DirectionalLightComponent, LightComponent);
    public:
        void BeginComponent() override;
        void EndComponent() override;

        void OnMsg_TransformChanged() override;
        sptr<rendering::RenderObject> m_gizmos;
    };
}