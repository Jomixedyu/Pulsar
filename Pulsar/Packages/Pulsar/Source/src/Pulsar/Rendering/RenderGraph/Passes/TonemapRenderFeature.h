#pragma once
#include "RenderFeature.h"
#include <Pulsar/Assets/Material.h>

namespace pulsar
{
    class TonemapRenderFeature : public RenderFeature
    {
    public:
        void OnRecord(RenderGraph& graph, RenderFrameData& frameData) override;

    private:
        void EnsureMaterial();

        RCPtr<Material> m_material;
    };
}