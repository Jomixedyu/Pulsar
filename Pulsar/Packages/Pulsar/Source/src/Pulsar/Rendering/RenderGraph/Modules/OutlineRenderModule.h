#pragma once
#include "MeshRenderModule.h"
#include <Pulsar/Rendering/RenderGraph/RenderGraph.h>

namespace pulsar
{
    class OutlineRenderModule : public MeshRenderModule
    {
    public:
        void OnRecord(RenderGraph& graph, RenderFrameData& frameData) override;
    };
}
