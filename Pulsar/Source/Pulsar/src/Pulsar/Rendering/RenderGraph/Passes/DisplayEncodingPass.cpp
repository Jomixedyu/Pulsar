#include "DisplayEncodingPass.h"
#include <Pulsar/AssetManager.h>
#include <Pulsar/Assets/Shader.h>

namespace pulsar
{
    DisplayEncodingPass::DisplayEncodingPass()
        : PostProcessPass(nullptr)
    {
    }

    void DisplayEncodingPass::EnsureMaterial()
    {
        if (m_material)
            return;

        auto shader = AssetManager::Get()->LoadAsset<Shader>("Engine/Shaders/DisplayEncoding");
        if (!shader)
            return;

        m_material = Material::StaticCreate(shader);
        m_material->CreateGPUResource();
    }

    void DisplayEncodingPass::ReadSettings(const VolumeStack& stack)
    {
        m_settings = stack.GetComponent<DisplayEncodingSettings>();
    }

    void DisplayEncodingPass::PrepareMaterial(CameraComponent* cam)
    {
        EnsureMaterial();
    }
}
