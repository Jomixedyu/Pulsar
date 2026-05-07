#pragma once
#include <Pulsar/Rendering/RenderGraph/RenderGraph.h>
#include <gfx/GFXDescriptorSet.h>

namespace gfx { class GFXTexture2DView; }

namespace pulsar
{
    class CameraComponent;
    class World;
    class PerPassResources;

    class OutlinePass
    {
    public:
        ~OutlinePass() { Destroy(); }
        void Initialize(PerPassResources* perPass);
        void Destroy();

        RGTextureHandle AddToGraph(RenderGraph& graph, RGTextureHandle hFinal,
                                   CameraComponent* cam, World* world,
                                   PerPassResources* perPass);

    private:
        gfx::GFXDescriptorSet_sp m_perPassSet;
    };
}
