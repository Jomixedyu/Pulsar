#pragma once
#include "RenderGraph.h"
#include "RenderPipelineFrameData.h"

namespace pulsar
{
    class RenderPipeline
    {
    public:
        virtual ~RenderPipeline() = default;
        virtual void OnRecord(RenderGraph& graph, RenderFrameData& frameData) = 0;
    };
}
