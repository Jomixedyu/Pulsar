#pragma once
#include <Pulsar/Rendering/RenderGraph/RenderGraph.h>
#include <Pulsar/Rendering/RenderGraph/RenderPipelineFrameData.h>

namespace pulsar
{
    class RenderModule
    {
    public:
        virtual ~RenderModule() = default;

        virtual void Initialize() {}
        virtual void Destroy() {}
        virtual void OnRecord(RenderGraph& graph, RenderFrameData& frameData) = 0;
    };
}