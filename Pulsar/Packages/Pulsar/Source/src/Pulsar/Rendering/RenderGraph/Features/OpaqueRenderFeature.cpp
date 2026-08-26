#include "OpaqueRenderFeature.h"
#include <Pulsar/Rendering/RenderGraph/Modules/OpaqueRenderModule.h>
#include <Pulsar/Rendering/RenderGraph/Pipelines/SceneCaptureFrameData.h>
#include <Pulsar/Rendering/RenderGraph/RenderPipelineFrameData.h>

namespace pulsar
{
    class OpaqueRenderFeature final : public RenderFeature
    {
    public:
        void OnCreateResource() override { m_module.Initialize(); }
        void OnDestroyResource() override { m_module.Destroy(); }
        void OnRecord(RenderGraph& graph, RenderFrameData& frameData) override
        {
            if (auto* resolve = frameData.Get<SceneResolveTargetFrameData>())
                m_module.SetResolveTargetView(resolve->TargetView);
            m_module.OnRecord(graph, frameData);
        }
    private:
        OpaqueRenderModule m_module;
    };

    RenderFeatureFactory OpaqueRenderFeatureSettings::BuildRenderFeatureFactory() const
    {
        return []() -> std::unique_ptr<RenderFeature>
        {
            return std::make_unique<OpaqueRenderFeature>();
        };
    }
}