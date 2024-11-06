#pragma once
#include "LightComponent.h"
#include "Pulsar/Scene.h"

namespace pulsar
{
    class SkyLightComponent : public LightComponent
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::SkyLightComponent, LightComponent);
        CORELIB_CLASS_ATTR(new CategoryAttribute("Light"));
    public:
    protected:
        void BeginComponent() override;
        void EndComponent() override;
        void OnIntensityChanged() override;
        void OnLightColorChanged() override;
        void OnEnvironmentChanged();
        void PostEditChange(FieldInfo* info) override;

        std::unique_ptr<SkyLightSceneInfo> m_sceneInfo;

        CORELIB_REFL_DECL_FIELD(m_environment)
        RCPtr<Texture2D> m_environment;
    };
}