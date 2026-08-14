#pragma once
#include <Pulsar/Rendering/RenderGraph/RenderPipeline.h>
#include <Pulsar/Rendering/RenderGraph/Pipelines/ViewPipelineRenderData.h>
#include <Pulsar/Rendering/RenderGraph/Features/RenderFeatureProxy.h>
#include <Pulsar/Rendering/RenderGraph/Modules/OpaqueRenderModule.h>
#include <Pulsar/Rendering/RenderGraph/Modules/TranslucencyRenderModule.h>
#include <Pulsar/Rendering/RenderGraph/Modules/OutlineRenderModule.h>
#include <Pulsar/Rendering/RenderGraph/Modules/BloomRenderModule.h>
#include <Pulsar/Rendering/RenderGraph/Modules/RenderModule.h>
#include <Pulsar/Rendering/RenderGraph/Modules/GizmoOverlayRenderModule.h>
#include <Pulsar/Rendering/PerPassData.h>
#include <memory>
#include <vector>

namespace pulsar
{
    class ViewPipeline final : public RenderPipeline
    {
    public:
        ViewPipeline();
        ~ViewPipeline() override;

        void OnCreateResource(const ViewPipelineRenderData& data);
        void ApplyRenderData(const ViewPipelineRenderData& data);
        void OnRecord(RenderGraph& graph, RenderFrameData& frameData) override;

    private:
        void RebuildFeatureProxies(const ViewPipelineRenderData& data);
        void RecordFeatures(RenderGraph& graph, RenderFrameData& frameData);

        OpaqueRenderModule m_opaqueModule;
        OutlineRenderModule m_outlineModule;
        TranslucencyRenderModule m_translucencyModule;
        std::vector<std::unique_ptr<RenderModule>> m_postProcessRenderModules;
        GizmoOverlayRenderModule m_gizmoOverlayModule;
        std::vector<std::unique_ptr<RenderFeatureProxy>> m_featureProxies;
    };
}
