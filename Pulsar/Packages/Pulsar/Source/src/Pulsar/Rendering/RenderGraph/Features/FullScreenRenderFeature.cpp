#include "FullScreenRenderFeature.h"
#include <Pulsar/Rendering/RenderGraph/Passes/BlitPass.h>
#include <Pulsar/Rendering/RenderGraph/Pipelines/SceneCaptureFrameData.h>
#include <Pulsar/Rendering/RenderGraph/RenderPipelineFrameData.h>
#include <Pulsar/Rendering/MaterialProxy.h>

namespace pulsar
{
    class FullScreenRenderFeature final : public RenderFeature
    {
    public:
        explicit FullScreenRenderFeature(const std::shared_ptr<MaterialProxy>& material)
            : m_material(material)
        {
        }

        void OnRecord(RenderGraph& graph, RenderFrameData& frameData) override
        {
            auto* capture = frameData.Get<SceneCaptureFrameData>();
            auto* postProcess = frameData.Get<ScenePostProcessFrameData>();
            if (!capture || !capture->view || !postProcess)
                return;

            auto* gpu = frameData.Get<SceneCaptureGpuFrameData>();
            auto result = BlitPass::AddToGraph(graph, {
                .Name = "FullScreen",
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

    RenderFeatureFactory FullScreenRenderFeatureSettings::BuildRenderFeatureFactory() const
    {
        if (!m_material)
            return {};

        if (!m_material->IsCreatedGPUResource())
            m_material->CreateGPUResource();

        auto materialProxy = m_material->GetRenderProxy();
        if (!materialProxy)
            return {};

        return [materialProxy]() -> std::unique_ptr<RenderFeature>
        {
            return std::make_unique<FullScreenRenderFeature>(materialProxy);
        };
    }

    void FullScreenRenderFeatureSettings::CollectAssetDependencies(array_list<guid_t>& dependencies)
    {
        if (m_material)
        {
            auto guid = m_material.GetGuid();
            if (!guid.is_empty())
                dependencies.push_back(guid);
        }
    }
}
