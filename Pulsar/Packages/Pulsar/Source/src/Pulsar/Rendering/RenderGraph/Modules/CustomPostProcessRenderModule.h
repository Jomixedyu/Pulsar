#pragma once
#include "RenderModule.h"
#include "../Pipelines/SceneCaptureFrameData.h"

namespace pulsar
{
    class CustomPostProcessRenderModule : public RenderModule
    {
    public:
        void OnRecord(RenderGraph& graph, RenderFrameData& frameData) override;
    };
}