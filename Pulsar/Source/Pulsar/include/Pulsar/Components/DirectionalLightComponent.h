#pragma once
#include "LightComponent.h"
#include "Pulsar/Rendering/RenderObject.h"
#include "Pulsar/Scene.h"

namespace pulsar
{
    class DirectionalLightComponent : public LightComponent
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::DirectionalLightComponent, LightComponent);
        CORELIB_CLASS_ATTR(new CategoryAttribute("Light"));
    public:
        void BeginComponent() override;
        void EndComponent() override;


    protected:

        void PostEditChange(FieldInfo* info) override;

        void OnIntensityChanged() override;
        void OnLightColorChanged() override;

        void OnTransformChanged() override;
        SPtr<rendering::RenderObject> m_gizmos;

        std::unique_ptr<DirectionalLightSceneInfo> m_sceneInfo;
    };
}