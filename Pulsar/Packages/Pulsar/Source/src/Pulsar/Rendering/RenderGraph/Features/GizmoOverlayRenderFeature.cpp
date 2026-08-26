#include "GizmoOverlayRenderFeature.h"
#include <Pulsar/Rendering/RenderGraph/Modules/GizmoOverlayRenderModule.h>
#include <Pulsar/Rendering/RenderGraph/Pipelines/SceneCaptureFrameData.h>
#include <Pulsar/Rendering/SceneView.h>

namespace pulsar
{
    class GizmoOverlayRenderFeature final : public RenderFeature
    {
    public:
        void OnCreateResource() override { m_module.Initialize(); }
        void OnDestroyResource() override { m_module.Destroy(); }
        void OnRecord(RenderGraph& graph, RenderFrameData& frameData) override
        {
            auto* capture = frameData.Get<SceneCaptureFrameData>();
            if (!capture || !capture->view || !capture->view->GizmoPassEnabled)
                return;
            m_module.OnRecord(graph, frameData);
        }
    private:
        GizmoOverlayRenderModule m_module;
    };

    RenderFeatureFactory GizmoOverlayRenderFeatureSettings::BuildRenderFeatureFactory() const
    {
        return []() -> std::unique_ptr<RenderFeature>
        {
            return std::make_unique<GizmoOverlayRenderFeature>();
        };
    }
}