#include "PostProcessRenderFeature.h"
#include <Pulsar/Rendering/RenderGraph/Modules/TonemapRenderModule.h>
#include <Pulsar/Rendering/RenderGraph/Modules/CustomPostProcessRenderModule.h>
#include <Pulsar/Rendering/RenderGraph/Modules/ColorGradingRenderModule.h>
#include <Pulsar/Rendering/RenderGraph/Modules/BloomRenderModule.h>
#include <Pulsar/Rendering/RenderGraph/Modules/DisplayEncodingRenderModule.h>
#include <Pulsar/Rendering/RenderGraph/Pipelines/SceneCaptureFrameData.h>
#include <vector>

namespace pulsar
{
    class PostProcessRenderFeature final : public RenderFeature
    {
    public:
        void OnCreateResource() override
        {
            m_modules.push_back(std::make_unique<TonemapRenderModule>());
            m_modules.push_back(std::make_unique<CustomPostProcessRenderModule>());
            m_modules.push_back(std::make_unique<ColorGradingRenderModule>());
            m_modules.push_back(std::make_unique<BloomRenderModule>());
            m_modules.push_back(std::make_unique<DisplayEncodingRenderModule>());
            for (auto& module : m_modules)
                module->Initialize();
        }

        void OnDestroyResource() override
        {
            for (auto& module : m_modules)
                module->Destroy();
            m_modules.clear();
        }

        void OnRecord(RenderGraph& graph, RenderFrameData& frameData) override
        {
            auto* postProcess = frameData.Get<ScenePostProcessFrameData>();
            if (!postProcess)
                return;
            for (auto& module : m_modules)
                module->OnRecord(graph, frameData);
        }

    private:
        std::vector<std::unique_ptr<RenderModule>> m_modules;
    };

    RenderFeatureFactory PostProcessRenderFeatureSettings::BuildRenderFeatureFactory() const
    {
        return []() -> std::unique_ptr<RenderFeature>
        {
            return std::make_unique<PostProcessRenderFeature>();
        };
    }
}