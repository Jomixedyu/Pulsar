#pragma once
#include "MeshRenderFeature.h"
#include <Pulsar/Rendering/RenderGraph/RenderGraph.h>

namespace pulsar
{
    class SceneCapture2DComponent;
    class PerPassResources;

    class OutlinePass : public MeshRenderFeature
    {
    public:
        RGTextureHandle AddToGraph(RenderGraph& graph,
                                   RGTextureHandle input,
                                   RGTextureHandle output,
                                   SceneCapture2DComponent* capture2D,
                                   PerPassResources* perPass) override;
    };
}
