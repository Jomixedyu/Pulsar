#pragma once
#include "MeshRenderModule.h"

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
                                          const RenderCaptureContext& ctx);
    };
}