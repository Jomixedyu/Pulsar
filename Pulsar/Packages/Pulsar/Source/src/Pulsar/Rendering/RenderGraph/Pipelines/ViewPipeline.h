#pragma once
#include <Pulsar/Rendering/RenderGraph/RenderPipeline.h>
#include <Pulsar/Rendering/RenderGraph/Pipelines/ViewPipelineRenderData.h>
#include <Pulsar/Rendering/RenderGraph/Features/RenderFeature.h>
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
        bool HasSameFeatures(const ViewPipelineRenderData& data) const;
    private:
        void RebuildFeatures(const ViewPipelineRenderData& data);
        void RecordFeatures(RenderGraph& graph, RenderFrameData& frameData);

        std::vector<std::unique_ptr<RenderFeature>> m_features;
        std::vector<Type*> m_featureTypes;
    };
}
