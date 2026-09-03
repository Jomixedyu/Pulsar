#include "TonemapRenderFeature.h"
#include <Pulsar/AssetManager.h>
#include <Pulsar/Assets/Shader.h>
#include <Pulsar/Assets/TonemappingSettings.h>
#include <Pulsar/Rendering/RenderGraph/Passes/BlitPass.h>
#include <Pulsar/Rendering/RenderGraph/Pipelines/SceneCaptureFrameData.h>
#include <Pulsar/Rendering/RenderGraph/RenderPipelineFrameData.h>
#include <Pulsar/Rendering/SceneView.h>
#include <Pulsar/Rendering/MaterialProxy.h>
#include <Pulsar/Rendering/ShaderPropertyValue.h>

namespace pulsar
{
    class TonemapRenderFeature final : public RenderFeature
    {
    public:
        explicit TonemapRenderFeature(const SPtr<ShaderConfig>& shaderConfig, guid_t shaderGuid)
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

            auto* snapshot = capture->view->PostProcessStack.GetComponent<TonemappingRenderSnapshot>();
            if (!snapshot || !snapshot->Enabled)
                return;

            ShaderPropertyRenderData parameters;
            ShaderPropertyValue mode;
            mode.SetValue(static_cast<int>(snapshot->Mode));
            parameters.Constants.emplace("_TonemappingMode", mode);
            m_material->ApplyRenderData(std::move(parameters));

            auto* gpu = frameData.Get<SceneCaptureGpuFrameData>();
            auto result = BlitPass::AddToGraph(graph, {
                .Name = "PostProcess_Tonemap",
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

    RenderFeatureFactory TonemapRenderFeatureSettings::BuildRenderFeatureFactory() const
    {
        auto shader = AssetManager::Get()->LoadAsset<Shader>("Pulsar/Shaders/Tonemap");
        SPtr<ShaderConfig> shaderConfig = shader ? shader->GetConfig() : nullptr;
        guid_t shaderGuid = shader ? shader->GetAssetGuid() : guid_t{};
        return [shaderConfig, shaderGuid]() -> std::unique_ptr<RenderFeature>
        {
            return std::make_unique<TonemapRenderFeature>(shaderConfig, shaderGuid);
        };
    }
}
