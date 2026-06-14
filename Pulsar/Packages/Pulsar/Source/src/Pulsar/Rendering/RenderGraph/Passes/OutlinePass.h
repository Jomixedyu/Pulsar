#pragma once
#include "MeshRenderFeature.h"
#include <Pulsar/Rendering/RenderGraph/RenderGraph.h>

namespace pulsar
{
    class PerPassResources;

    class OutlinePass : public MeshRenderFeature
    {
    public:
        RGTextureHandle AddToGraph(RenderGraph& graph,
                                   RGTextureHandle input,
                                   RGTextureHandle output,
                                   const RenderCaptureContext& ctx,
                                   PerPassResources* perPass) override;
    };
}
