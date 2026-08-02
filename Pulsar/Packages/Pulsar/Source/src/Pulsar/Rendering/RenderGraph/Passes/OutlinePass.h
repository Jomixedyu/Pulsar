#pragma once
#include "MeshRenderFeature.h"
#include <Pulsar/Rendering/RenderGraph/RenderGraph.h>

namespace pulsar
{
    class OutlinePass : public MeshRenderFeature
    {
    public:
        RGTextureHandle AddToGraph(RenderGraph& graph,
                                   RGTextureHandle input,
                                   RGTextureHandle output,
                                   const RenderCaptureContext& ctx) override;
    };
}
