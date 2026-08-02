#pragma once
#include "MeshRenderFeature.h"
#include <Pulsar/Rendering/RenderGraph/RenderGraph.h>

namespace pulsar
{
    class TranslucencyPass : public MeshRenderFeature
    {
    public:
        void SetOpaqueColor(RGTextureHandle h) { m_hOpaqueColor = h; }
        void SetOpaqueDepth(RGTextureHandle h) { m_hOpaqueDepth = h; }

        RGTextureHandle AddToGraph(RenderGraph& graph,
                                   RGTextureHandle input,
                                   RGTextureHandle output,
                                   const RenderCaptureContext& ctx) override;

    private:
        RGTextureHandle m_hOpaqueColor;
        RGTextureHandle m_hOpaqueDepth;
    };
}
