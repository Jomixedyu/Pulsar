#pragma once
#include <Pulsar/Rendering/RenderGraph/RenderGraph.h>
#include <Pulsar/EngineMath.h>

namespace gfx { class GFXTexture2DView; }

namespace pulsar
{
    class CameraComponent;
    class World;
    class PerPassResources;

    class BasePass
    {
    public:
        RGTextureHandle AddToGraph(RenderGraph& graph, RGTextureHandle hFinal,
                                   CameraComponent* cam, World* world,
                                   PerPassResources* perPass,
                                   gfx::GFXTexture2DView* resolveTargetView = nullptr);
    };
}
