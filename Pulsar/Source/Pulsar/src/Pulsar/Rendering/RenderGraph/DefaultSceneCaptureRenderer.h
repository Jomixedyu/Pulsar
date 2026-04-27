#pragma once
#include "ScriptableCaptureRenderer.h"
#include "Passes/BasePass.h"
#include "Passes/RenderFeature.h"
#include "../PerPassResources.h"
#include <memory>
#include <vector>

namespace pulsar
{
    // Default implementation of ScriptableCaptureRenderer.
    // Assembles a forward rendering pipeline: BasePass -> registered features.
    class DefaultSceneCaptureRenderer : public ScriptableCaptureRenderer
    {
    public:
        DefaultSceneCaptureRenderer();
        ~DefaultSceneCaptureRenderer() override;

        void Render(RenderGraph& graph, const RenderCaptureContext& ctx) override;

    private:
        PerPassResources m_perPassResources;
        BasePass m_basePass;
        std::vector<std::unique_ptr<RenderFeature>> m_features;
    };

} // namespace pulsar
