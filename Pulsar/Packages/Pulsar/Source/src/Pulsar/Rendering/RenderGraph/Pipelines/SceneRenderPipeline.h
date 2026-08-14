#pragma once
#include <Pulsar/Rendering/RenderGraph/RenderGraph.h>

namespace pulsar
{
    class RenderScene;

    class SceneRenderPipeline
    {
    public:
        void OnRecord(RenderGraph& graph, RenderScene& scene, uint64_t frameIndex);
    };
}
