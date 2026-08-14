#pragma once

namespace pulsar
{
    class RenderGraph;
    class RenderFrameData;

    class RenderFeatureProxy
    {
    public:
        virtual ~RenderFeatureProxy() = default;

        virtual void OnCreateResource() {}
        virtual void OnDestroyResource() {}
        virtual void OnRecord(RenderGraph& graph, RenderFrameData& frameData) = 0;
    };
}
