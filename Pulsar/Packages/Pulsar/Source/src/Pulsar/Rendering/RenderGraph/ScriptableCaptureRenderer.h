#pragma once
#include "RenderGraph.h"

namespace pulsar
{
    class RenderScene;
    class SceneView;
    struct SceneViewData;

    struct RenderCaptureContext
    {
        const SceneViewData*   view       = nullptr;
        SceneView*             viewProxy  = nullptr;
        RenderScene*           scene      = nullptr;
        uint64_t               frameIndex = 0;
    };

    class ScriptableCaptureRenderer
    {
    public:
        virtual ~ScriptableCaptureRenderer() = default;
        virtual void Render(RenderGraph& graph, const RenderCaptureContext& ctx) = 0;
    };

} // namespace pulsar
