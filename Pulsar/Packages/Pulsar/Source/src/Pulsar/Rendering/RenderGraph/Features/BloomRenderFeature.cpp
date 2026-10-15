#include "BloomRenderFeature.h"
#include <Pulsar/AssetManager.h>
#include <Pulsar/Assets/Shader.h>
#include <Pulsar/Rendering/MaterialProxy.h>
#include <Pulsar/Rendering/ShaderProxy.h>
#include <Pulsar/Rendering/RenderGraph/Modules/BloomRenderModule.h>

namespace pulsar
{
    class BloomRenderFeature final : public RenderFeature
    {
    public:
        explicit BloomRenderFeature(const std::shared_ptr<ShaderProxy>& shaderProxy)
        {
            m_material = std::make_shared<MaterialProxy>();
            m_material->UpdateShader(shaderProxy, {}, ShaderPassRenderQueueType::Opaque, nullptr, nullptr);
            m_module = std::make_unique<BloomRenderModule>(m_material);
        }

        void OnCreateResource() override { m_module->Initialize(); }
        void OnDestroyResource() override { m_module->Destroy(); }
        void OnRecord(RenderGraph& graph, RenderFrameData& frameData) override
        {
            m_module->OnRecord(graph, frameData);
        }

    private:
        std::shared_ptr<MaterialProxy> m_material;
        std::unique_ptr<BloomRenderModule> m_module;
    };

    RenderFeatureFactory BloomRenderFeatureSettings::BuildRenderFeatureFactory() const
    {
        auto shader = AssetManager::Get()->LoadAsset<Shader>("Pulsar/Shaders/NapBloom");
        if (shader)
            shader->EnsureGPUResource();
        auto shaderProxy = shader ? shader->GetRenderProxy() : nullptr;
        return [shaderProxy]() -> std::unique_ptr<RenderFeature>
        {
            return std::make_unique<BloomRenderFeature>(shaderProxy);
        };
    }
}
