#pragma once
#include "MeshRenderFeature.h"
#include <Pulsar/Rendering/RenderGraph/RenderGraph.h>
#include <Pulsar/EngineMath.h>

namespace gfx { class GFXTexture2DView; }

namespace pulsar
{
    class OpaqueRenderFeature : public MeshRenderFeature
    {
    public:
        void SetResolveTargetView(gfx::GFXTexture2DView* view) { m_resolveTargetView = view; }
        void OnRecord(RenderGraph& graph, RenderFrameData& frameData) override;

    private:
        RGTextureHandle RecordOpaque(RenderGraph& graph,
                                         RGTextureHandle input,
                                         RGTextureHandle output,
                                         const RenderCaptureContext& ctx);

        gfx::GFXTexture2DView* m_resolveTargetView = nullptr;
    };
}