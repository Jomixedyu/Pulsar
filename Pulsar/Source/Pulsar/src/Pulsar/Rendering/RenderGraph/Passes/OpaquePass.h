#pragma once
#include <Pulsar/Rendering/RenderGraph/RenderGraph.h>
#include <Pulsar/EngineMath.h>
#include <gfx/GFXDescriptorSet.h>

namespace gfx { class GFXTexture2DView; }

namespace pulsar
{
    class CameraComponent;
    class World;
    class PerPassResources;

    class OpaquePass
    {
    public:
        ~OpaquePass() { Destroy(); }
        void Initialize(PerPassResources* perPass);
        void Destroy();

        RGTextureHandle AddToGraph(RenderGraph& graph, RGTextureHandle hFinal,
                                   CameraComponent* cam, World* world,
                                   PerPassResources* perPass,
                                   gfx::GFXTexture2DView* resolveTargetView = nullptr);

    private:
        gfx::GFXDescriptorSet_sp m_perPassSet;
    };
}
