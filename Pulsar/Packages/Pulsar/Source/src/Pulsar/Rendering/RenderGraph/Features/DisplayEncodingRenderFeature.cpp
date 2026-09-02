#include "DisplayEncodingRenderFeature.h"
#include <Pulsar/AssetManager.h>
#include <Pulsar/Assets/DisplayEncodingSettings.h>
#include <Pulsar/Assets/Shader.h>
#include <Pulsar/Rendering/RenderGraph/Passes/BlitPass.h>
#include <Pulsar/Rendering/RenderGraph/Pipelines/SceneCaptureFrameData.h>
#include <Pulsar/Rendering/RenderGraph/RenderPipelineFrameData.h>
#include <Pulsar/Rendering/SceneView.h>
#include <Pulsar/Rendering/MaterialProxy.h>

namespace pulsar
{
    class DisplayEncodingRenderFeature final : public RenderFeature
    {
    public:
        explicit DisplayEncodingRenderFeature(const SPtr<ShaderConfig>& shaderConfig, guid_t shaderGuid)
        {
            m_material = std::make_shared<MaterialProxy>();
            m_material->UpdateShader(shaderConfig, shaderGuid, {}, ShaderPassRenderQueueType::Opaque, nullptr, nullptr);
        }

        void OnRecord(RenderGraph& graph, RenderFrameData& frameData) override
        {
            auto* capture = frameData.Get<SceneCaptureFrameData>();
            auto* postProcess = frameData.Get<ScenePostProcessFrameData>();
            if (!capture || !capture->view || !postProcess)
                return;

            auto* snapshot = capture->view->PostProcessStack.GetComponent<DisplayEncodingRenderSnapshot>();
            if (!snapshot || !snapshot->Enabled)
                return;

            auto* gpu = frameData.Get<SceneCaptureGpuFrameData>();
            auto result = BlitPass::AddToGraph(graph, {
                .Name = "PostProcess_DisplayEncoding",
                .Source = postProcess->ActiveColor,
                .Destination = postProcess->AcquireTarget(),
                .Material = m_material,
                .ShaderPass = "PostProcess",
                .Interface = "RENDERER_IMAGEPROCESS",
                .SourceBindingName = "PP_InColor",
                .CameraBuffer = gpu ? gpu->CameraBuffer : nullptr,
                .WorldBuffer = gpu ? gpu->WorldBuffer : nullptr,
            });
            postProcess->PushColor(result);
        }

    private:
        std::shared_ptr<MaterialProxy> m_material;
    };

    RenderFeatureFactory DisplayEncodingRenderFeatureSettings::BuildRenderFeatureFactory() const
    {
        auto shader = AssetManager::Get()->LoadAsset<Shader>("Pulsar/Shaders/DisplayEncoding");
        SPtr<ShaderConfig> shaderConfig = shader ? shader->GetConfig() : nullptr;
        guid_t shaderGuid = shader ? shader->GetAssetGuid() : guid_t{};
        return [shaderConfig, shaderGuid]() -> std::unique_ptr<RenderFeature>
        {
            return std::make_unique<DisplayEncodingRenderFeature>(shaderConfig, shaderGuid);
        };
    }
}
