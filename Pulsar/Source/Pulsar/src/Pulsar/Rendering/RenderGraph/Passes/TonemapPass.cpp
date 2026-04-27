#include "TonemapPass.h"
#include <Pulsar/AssetManager.h>
#include <Pulsar/Assets/Shader.h>

namespace pulsar
{
    TonemapPass::TonemapPass()
        : PostProcessPass(nullptr)
    {
    }

    void TonemapPass::EnsureMaterial()
    {
        if (m_material)
            return;

        auto shader = AssetManager::Get()->LoadAsset<Shader>("Engine/Shaders/Tonemap");
        if (!shader)
            return;

        m_material = Material::StaticCreate(shader);
        m_material->CreateGPUResource();
    }

    void TonemapPass::ReadSettings(const VolumeStack& stack)
    {
        m_settings = stack.GetComponent<TonemappingSettings>();
    }

    void TonemapPass::PrepareMaterial(CameraComponent* cam)
    {
        EnsureMaterial();
        if (m_material && m_settings)
            m_material->SetIntScalar("_TonemappingMode", static_cast<int>(m_settings->m_mode));
    }
}
