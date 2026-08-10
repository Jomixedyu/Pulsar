#pragma once
#include "RenderModule.h"

namespace pulsar
{
    class CustomPostProcessRenderModule : public RenderModule
    {
    public:
        void OnRecord(RenderGraph& graph, RenderFrameData& frameData) override;
    };
}