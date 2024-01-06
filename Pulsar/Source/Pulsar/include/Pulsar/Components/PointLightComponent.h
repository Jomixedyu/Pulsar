#pragma once
#include "LightComponent.h"
#include "Pulsar/Rendering/RenderObject.h"

namespace pulsar
{
    class PointLightComponent : public LightComponent
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::PointLightComponent, LightComponent);
    public:
        void BeginComponent() override;
        void EndComponent() override;

        void OnMsg_TransformChanged() override;
        sptr<rendering::RenderObject> m_gizmos;
    };
}