#pragma once
#include "RenderFeature.h"

namespace pulsar
{
    class CustomPostProcessRenderFeature : public RenderFeature
    {
    public:
        void OnRecord(RenderGraph& graph, RenderFrameData& frameData) override;
    };
}