#include "Components/SkyLightComponent.h"

#include "Scene.h"

namespace pulsar
{

    void SkyLightComponent::BeginComponent()
    {
        base::BeginComponent();

        m_sceneInfo = std::make_unique<SkyLightSceneInfo>();
        GetRuntimeScene()->GetRuntimeEnvironment().AddSkyLight(m_sceneInfo.get());
        OnIntensityChanged();
        OnLightColorChanged();
    }
    void SkyLightComponent::EndComponent()
    {
        base::EndComponent();
        GetRuntimeScene()->GetRuntimeEnvironment().RemoveSkylLight(m_sceneInfo.get());
        m_sceneInfo.reset();
    }
    void SkyLightComponent::OnIntensityChanged()
    {
        base::OnIntensityChanged();
        m_sceneInfo->Intensity = m_intensity;
    }
    void SkyLightComponent::OnLightColorChanged()
    {
        base::OnLightColorChanged();
        m_sceneInfo->Color = m_lightColor;
    }
} // namespace pulsar
