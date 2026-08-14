#pragma once
#include "RenderModule.h"
#include "../Pipelines/SceneCaptureFrameData.h"
#include <Pulsar/Assets/Material.h>

namespace pulsar
{
    class DisplayEncodingRenderModule : public RenderModule
    {
    public:
        void OnRecord(RenderGraph& graph, RenderFrameData& frameData) override;

    private:
        void EnsureMaterial();

        RCPtr<Material> m_material;
    };
}