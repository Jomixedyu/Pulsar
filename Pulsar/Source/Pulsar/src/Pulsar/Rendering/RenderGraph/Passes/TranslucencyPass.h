#pragma once
#include <Pulsar/Rendering/RenderGraph/RenderGraph.h>
#include <gfx/GFXDescriptorSet.h>

namespace pulsar
{
    class CameraComponent;
    class World;
    class PerPassResources;

    class TranslucencyPass
    {
    public:
        ~TranslucencyPass() { Destroy(); }
        void Initialize(PerPassResources* perPass);
        void Destroy();

        RGTextureHandle AddToGraph(RenderGraph& graph, RGTextureHandle hFinal,
                                   CameraComponent* cam, World* world,
                                   PerPassResources* perPass,
                                   RGTextureHandle hOpaqueColor,
                                   RGTextureHandle hOpaqueDepth = {});

    private:
        gfx::GFXDescriptorSet_sp m_perPassSet;
    };
}
