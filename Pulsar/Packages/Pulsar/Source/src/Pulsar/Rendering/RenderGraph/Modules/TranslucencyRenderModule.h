#pragma once
#include "MeshRenderModule.h"
#include "../Pipelines/SceneCaptureFrameData.h"
#include <Pulsar/Rendering/RenderGraph/RenderGraph.h>

namespace pulsar
{
    class TranslucencyRenderModule : public MeshRenderModule
    {
    public:
        void SetOpaqueColor(RGTextureHandle h) { m_hOpaqueColor = h; }
        void SetOpaqueDepth(RGTextureHandle h) { m_hOpaqueDepth = h; }
        void OnRecord(RenderGraph& graph, RenderFrameData& frameData) override;

    private:
        RGTextureHandle RecordTranslucency(RenderGraph& graph,
                                               RGTextureHandle input,
                                               RGTextureHandle output,
                                               const SceneCaptureFrameData& capture,
                                               const std::shared_ptr<array_list<rendering::RenderObject_sp>>& visibleRenderers);

        RGTextureHandle m_hOpaqueColor;
        RGTextureHandle m_hOpaqueDepth;
    };
}
