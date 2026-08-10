#pragma once
#include "MeshRenderFeature.h"

namespace pulsar
{
    class GizmoOverlayRenderFeature : public MeshRenderFeature
    {
    public:
        void OnRecord(RenderGraph& graph, RenderFrameData& frameData) override;

    private:
        RGTextureHandle RecordOverlayPass(RenderGraph& graph,
                                          RGTextureHandle input,
                                          RGTextureHandle output,
                                          const RenderCaptureContext& ctx);
    };
}