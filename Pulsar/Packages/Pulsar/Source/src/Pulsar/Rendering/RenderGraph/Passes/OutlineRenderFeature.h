#pragma once
#include "MeshRenderFeature.h"
#include <Pulsar/Rendering/RenderGraph/RenderGraph.h>

namespace pulsar
{
    class OutlineRenderFeature : public MeshRenderFeature
    {
    public:
        void OnRecord(RenderGraph& graph, RenderFrameData& frameData) override;
    };
}
