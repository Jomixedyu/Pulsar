#pragma once
#include "MeshRenderModule.h"
#include "../Pipelines/SceneCaptureFrameData.h"

namespace pulsar
{
    class GizmoOverlayRenderModule : public MeshRenderModule
    {
    public:
        void OnRecord(RenderGraph& graph, RenderFrameData& frameData) override;

    private:
        RGTextureHandle RecordOverlayPass(RenderGraph& graph,
                                          RGTextureHandle input,
                                          RGTextureHandle output,
                                          const SceneCaptureFrameData& capture,
                                          const std::shared_ptr<array_list<rendering::RenderObject_sp>>& visibleRenderers);
    };
}
