#pragma once
#include "MeshRenderFeature.h"
#include <Pulsar/Rendering/RenderGraph/RenderGraph.h>

namespace pulsar
{
    class TranslucencyRenderFeature : public MeshRenderFeature
    {
    public:
        void SetOpaqueColor(RGTextureHandle h) { m_hOpaqueColor = h; }
        void SetOpaqueDepth(RGTextureHandle h) { m_hOpaqueDepth = h; }
        void OnRecord(RenderGraph& graph, RenderFrameData& frameData) override;

    private:
        RGTextureHandle RecordTranslucency(RenderGraph& graph,
                                               RGTextureHandle input,
                                               RGTextureHandle output,
                                               const RenderCaptureContext& ctx);

        RGTextureHandle m_hOpaqueColor;
        RGTextureHandle m_hOpaqueDepth;
    };
}