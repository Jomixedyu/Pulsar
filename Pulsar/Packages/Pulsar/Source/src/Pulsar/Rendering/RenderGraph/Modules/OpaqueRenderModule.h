#pragma once
#include "MeshRenderModule.h"
#include "../Pipelines/SceneCaptureFrameData.h"
#include <Pulsar/Rendering/RenderGraph/RenderGraph.h>
#include <Pulsar/EngineMath.h>

namespace gfx { class GFXTexture2DView; }

namespace pulsar
{
    class OpaqueRenderModule : public MeshRenderModule
    {
    public:
        void SetResolveTargetView(gfx::GFXTexture2DView* view) { m_resolveTargetView = view; }
        void OnRecord(RenderGraph& graph, RenderFrameData& frameData) override;

    private:
        RGTextureHandle RecordOpaque(RenderGraph& graph,
                                           RGTextureHandle input,
                                           RGTextureHandle output,
                                           const SceneCaptureFrameData& capture,
                                           const std::shared_ptr<array_list<rendering::RenderObject_sp>>& visibleRenderers);

        gfx::GFXTexture2DView* m_resolveTargetView = nullptr;
    };
}
