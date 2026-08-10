#pragma once
#include "ScriptableCaptureRenderer.h"
#include "Passes/OpaqueRenderFeature.h"
#include "Passes/TranslucencyRenderFeature.h"
#include "Passes/OutlineRenderFeature.h"
#include "Passes/BloomRenderFeature.h"
#include "Passes/RenderFeature.h"
#include "Passes/GizmoOverlayRenderFeature.h"
#include "../PerPassData.h"
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
        OpaqueRenderFeature m_opaqueFeature;
        OutlineRenderFeature m_outlineFeature;
        TranslucencyRenderFeature m_translucencyFeature;
        std::vector<std::unique_ptr<RenderFeature>> m_postProcessRenderFeatures;
        GizmoOverlayRenderFeature m_gizmoOverlayFeature;
    };

} // namespace pulsar
