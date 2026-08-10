#pragma once
#include "RenderModule.h"
#include <Pulsar/Assets/Material.h>

namespace pulsar
{
    class TonemapRenderModule : public RenderModule
    {
    public:
        void OnRecord(RenderGraph& graph, RenderFrameData& frameData) override;

    private:
        void EnsureMaterial();

        RCPtr<Material> m_material;
    };
}