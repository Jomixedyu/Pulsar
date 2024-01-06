#pragma once
#include "LightComponent.h"
#include "Pulsar/Scene.h"

namespace pulsar
{
    class SkyLightComponent : public LightComponent
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::SkyLightComponent, LightComponent);
    public:
    protected:
        void BeginComponent() override;
        void EndComponent() override;
        void OnIntensityChanged() override;
        void OnLightColorChanged() override;

        std::unique_ptr<SkyLightSceneInfo> m_sceneInfo;
    };
}