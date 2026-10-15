#include "DisplayEncodingRenderFeature.h"
#include <Pulsar/AssetManager.h>
#include <Pulsar/Assets/DisplayEncodingSettings.h>
#include <Pulsar/Assets/Shader.h>
#include <Pulsar/Rendering/RenderGraph/Passes/BlitPass.h>
#include <Pulsar/Rendering/RenderGraph/Pipelines/SceneCaptureFrameData.h>
#include <Pulsar/Rendering/RenderGraph/RenderPipelineFrameData.h>
#include <Pulsar/Rendering/SceneViewProxy.h>
#include <Pulsar/Rendering/MaterialProxy.h>
#include <Pulsar/Rendering/ShaderProxy.h>

namespace pulsar
{
    class DisplayEncodingRenderFeature final : public RenderFeature
    {
    public:
        explicit DisplayEncodingRenderFeature(const std::shared_ptr<ShaderProxy>& shaderProxy)
        {
            m_material = std::make_shared<MaterialProxy>();
            m_material->UpdateShader(shaderProxy, {}, ShaderPassRenderQueueType::Opaque, nullptr, nullptr);
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
        if (shader)
            shader->EnsureGPUResource();
        auto shaderProxy = shader ? shader->GetRenderProxy() : nullptr;
        return [shaderProxy]() -> std::unique_ptr<RenderFeature>
        {
            return std::make_unique<DisplayEncodingRenderFeature>(shaderProxy);
        };
    }
}
