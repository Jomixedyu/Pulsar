#pragma once
#include "RenderFeature.h"
#include <gfx/GFXDescriptorSet.h>

namespace pulsar
{
    class GizmoOverlayPass : public RenderFeature
    {
    public:
        ~GizmoOverlayPass() { Destroy(); }
        void Initialize(PerPassResources* perPass);
        void Destroy();

        RGTextureHandle AddToGraph(RenderGraph& graph,
                                   RGTextureHandle input,
                                   RGTextureHandle output,
                                   CameraComponent* cam,
                                   PerPassResources* perPass) override;

    private:
        gfx::GFXDescriptorSet_sp m_perPassSet;
    };
}
