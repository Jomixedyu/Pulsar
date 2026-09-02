#include "ColorGradingRenderFeature.h"
#include <Pulsar/AssetManager.h>
#include <Pulsar/Assets/ColorGradingSettings.h>
#include <Pulsar/Assets/Shader.h>
#include <Pulsar/Rendering/RenderGraph/Passes/BlitPass.h>
#include <Pulsar/Rendering/RenderGraph/Pipelines/SceneCaptureFrameData.h>
#include <Pulsar/Rendering/RenderGraph/RenderPipelineFrameData.h>
#include <Pulsar/Rendering/SceneView.h>
#include <Pulsar/Rendering/MaterialProxy.h>
#include <Pulsar/Rendering/ShaderPropertyValue.h>

namespace pulsar
{
    class ColorGradingRenderFeature final : public RenderFeature
    {
    public:
        explicit ColorGradingRenderFeature(const SPtr<ShaderConfig>& shaderConfig, guid_t shaderGuid)
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

            auto* snapshot = capture->view->PostProcessStack.GetComponent<ColorGradingRenderSnapshot>();
            if (!snapshot || !snapshot->Enabled || !snapshot->LutTexture.IsValid())
                return;

            ShaderPropertyRenderData parameters;
            ShaderPropertyValue intensity;
            intensity.SetValue(snapshot->Intensity);
            parameters.Constants.emplace("_Intensity", intensity);

            ShaderPropertyValue lutSize;
            lutSize.SetValue(snapshot->LutSize);
            parameters.Constants.emplace("_LUTSize", lutSize);

            ShaderPropertyValue colorSpace;
            colorSpace.SetValue(static_cast<int>(snapshot->ColorSpace));
            parameters.Constants.emplace("_ColorSpace", colorSpace);
            parameters.Textures.emplace("_LUTTex", snapshot->LutTexture);
            m_material->ApplyRenderData(std::move(parameters));

            auto* gpu = frameData.Get<SceneCaptureGpuFrameData>();
            auto result = BlitPass::AddToGraph(graph, {
                .Name = "PostProcess_LUT",
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

    RenderFeatureFactory ColorGradingRenderFeatureSettings::BuildRenderFeatureFactory() const
    {
        auto shader = AssetManager::Get()->LoadAsset<Shader>("Pulsar/Shaders/LUT");
        SPtr<ShaderConfig> shaderConfig = shader ? shader->GetConfig() : nullptr;
        guid_t shaderGuid = shader ? shader->GetAssetGuid() : guid_t{};
        return [shaderConfig, shaderGuid]() -> std::unique_ptr<RenderFeature>
        {
            return std::make_unique<ColorGradingRenderFeature>(shaderConfig, shaderGuid);
        };
    }
}
