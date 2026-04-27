#pragma once
#include <Pulsar/Rendering/RenderGraph/RenderGraph.h>
#include <Pulsar/EngineMath.h>

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
                                   PerPassResources* perPass);
    };
}
