#pragma once
#include "MeshRenderFeature.h"
#include <Pulsar/Rendering/RenderGraph/RenderGraph.h>
#include <Pulsar/EngineMath.h>

namespace gfx { class GFXTexture2DView; }

namespace pulsar
{
    class SceneCapture2DComponent;
    class PerPassResources;

    class OpaquePass : public MeshRenderFeature
    {
    public:
        void SetResolveTargetView(gfx::GFXTexture2DView* view) { m_resolveTargetView = view; }

        RGTextureHandle AddToGraph(RenderGraph& graph,
                                   RGTextureHandle input,
                                   RGTextureHandle output,
                                   SceneCapture2DComponent* capture2D,
                                   PerPassResources* perPass) override;

    private:
        gfx::GFXTexture2DView* m_resolveTargetView = nullptr;
    };
}
