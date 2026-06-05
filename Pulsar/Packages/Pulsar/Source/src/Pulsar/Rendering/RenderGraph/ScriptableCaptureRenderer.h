#pragma once
#include "RenderGraph.h"

namespace pulsar
{
    class World;
    class SceneCaptureComponent;

    struct RenderCaptureContext
    {
        SceneCaptureComponent* capture    = nullptr;
        World*                 world      = nullptr;
        uint64_t               frameIndex = 0;
    };

    class ScriptableCaptureRenderer
    {
    public:
        virtual ~ScriptableCaptureRenderer() = default;
        virtual void Render(RenderGraph& graph, const RenderCaptureContext& ctx) = 0;
    };

} // namespace pulsar
