#pragma once
#include "ScriptableCaptureRenderer.h"
#include "Passes/OpaquePass.h"
#include "Passes/TranslucencyPass.h"
#include "Passes/RenderFeature.h"
#include "Passes/GizmoOverlayPass.h"
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
        OpaquePass m_opaquePass;
        TranslucencyPass m_translucencyPass;
        std::vector<std::unique_ptr<RenderFeature>> m_postProcessFeatures;
        GizmoOverlayPass m_gizmoOverlayPass;
    };

} // namespace pulsar
