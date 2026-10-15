#include "OutlineRenderFeature.h"
#include <Pulsar/Rendering/RenderGraph/Features/RenderFeature.h>
#include <Pulsar/Rendering/RenderGraph/Modules/OutlineRenderModule.h>

namespace pulsar
{
    class OutlineRenderFeature final : public RenderFeature
    {
    public:
        void OnCreateResource() override { m_module.Initialize(); }
        void OnDestroyResource() override { m_module.Destroy(); }
        void OnRecord(RenderGraph& graph, RenderFrameData& frameData) override
        {
            m_module.OnRecord(graph, frameData);
        }

    private:
        OutlineRenderModule m_module;
    };

    RenderFeatureFactory OutlineRenderFeatureSettings::BuildRenderFeatureFactory() const
    {
        return []() -> std::unique_ptr<RenderFeature>
        {
            return std::make_unique<OutlineRenderFeature>();
        };
    }
}