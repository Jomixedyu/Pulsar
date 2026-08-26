#pragma once

namespace pulsar
{
    class RenderGraph;
    class RenderFrameData;

    class RenderFeature
    {
    public:
        virtual ~RenderFeature() = default;

        virtual void OnCreateResource() {}
        virtual void OnDestroyResource() {}
        virtual void OnRecord(RenderGraph& graph, RenderFrameData& frameData) = 0;
    };
}
