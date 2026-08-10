#pragma once
#include "ScriptableCaptureRenderer.h"
#include "Modules/OpaqueRenderModule.h"
#include "Modules/TranslucencyRenderModule.h"
#include "Modules/OutlineRenderModule.h"
#include "Modules/BloomRenderModule.h"
#include "Modules/RenderModule.h"
#include "Modules/GizmoOverlayRenderModule.h"
#include "../PerPassData.h"
#include <memory>
#include <vector>

namespace pulsar
{
    // Default implementation of ScriptableCaptureRenderer.
    // Assembles a forward rendering pipeline: BasePass -> registered modules.
    class DefaultSceneCaptureRenderer : public ScriptableCaptureRenderer
    {
    public:
        DefaultSceneCaptureRenderer();
        ~DefaultSceneCaptureRenderer() override;

        void Render(RenderGraph& graph, const RenderCaptureContext& ctx) override;

    private:
        OpaqueRenderModule m_opaqueModule;
        OutlineRenderModule m_outlineModule;
        TranslucencyRenderModule m_translucencyModule;
        std::vector<std::unique_ptr<RenderModule>> m_postProcessRenderModules;
        GizmoOverlayRenderModule m_gizmoOverlayModule;
    };

} // namespace pulsar
