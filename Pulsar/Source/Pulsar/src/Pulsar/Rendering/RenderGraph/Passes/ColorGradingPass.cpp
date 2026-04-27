#include "ColorGradingPass.h"
#include <Pulsar/AssetManager.h>
#include <Pulsar/Assets/Shader.h>

namespace pulsar
{
    ColorGradingPass::ColorGradingPass()
        : PostProcessPass(nullptr)
    {
    }

    void ColorGradingPass::EnsureMaterial()
    {
        if (m_material)
            return;

        auto shader = AssetManager::Get()->LoadAsset<Shader>("Engine/Shaders/LUT");
        if (!shader)
            return;

        m_material = Material::StaticCreate(shader);
        m_material->CreateGPUResource();
    }

    void ColorGradingPass::ReadSettings(const VolumeStack& stack)
    {
        m_settings = stack.GetComponent<ColorGradingSettings>();
    }

    bool ColorGradingPass::IsEnabled() const
    {
        return m_settings != nullptr && m_settings->m_enabled && m_settings->m_lutTexture != nullptr;
    }

    void ColorGradingPass::PrepareMaterial(CameraComponent* cam)
    {
        EnsureMaterial();
        if (!m_material || !m_settings)
            return;

        m_material->SetTexture("_LUTTex", m_settings->m_lutTexture);
        m_material->SetFloat("_Intensity", m_settings->m_intensity);
        m_material->SetIntScalar("_LUTSize", m_settings->m_lutSize);
        m_material->SetIntScalar("_ColorSpace", static_cast<int>(m_settings->m_colorSpace));
    }
}
