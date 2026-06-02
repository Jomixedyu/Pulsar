#pragma once
#include "MeshRenderFeature.h"

namespace pulsar
{
    class GizmoOverlayPass : public MeshRenderFeature
    {
    public:
        RGTextureHandle AddToGraph(RenderGraph& graph,
                                   RGTextureHandle input,
                                   RGTextureHandle output,
                                   SceneCapture2DComponent* capture2D,
                                   PerPassResources* perPass) override;
    };
}
