#include "GammaCorrectionPass.h"
#include <Pulsar/AssetManager.h>
#include <Pulsar/Assets/Shader.h>

namespace pulsar
{
    GammaCorrectionPass::GammaCorrectionPass()
        : PostProcessPass(nullptr)
    {
    }

    void GammaCorrectionPass::EnsureMaterial()
    {
        if (m_material)
            return;

        auto shader = AssetManager::Get()->LoadAsset<Shader>("Engine/Shaders/Gamma");
        if (!shader)
            return;

        m_material = Material::StaticCreate(shader);
        m_material->CreateGPUResource();
    }

    void GammaCorrectionPass::ReadSettings(const VolumeStack& stack)
    {
        m_settings = stack.GetComponent<GammaCorrectionSettings>();
    }

    void GammaCorrectionPass::PrepareMaterial(CameraComponent* cam)
    {
        EnsureMaterial();
        if (m_material && m_settings)
            m_material->SetFloat("_Gamma", m_settings->m_gamma);
    }
}
