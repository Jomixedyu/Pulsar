#pragma once
#include "ScriptableCaptureRenderer.h"
#include "../PerPassResources.h"

namespace pulsar
{
    // Default implementation of ScriptableCaptureRenderer.
    // Declares a single BasePass that renders all RenderObjects in the world.
    // Shadow and PostProcess passes are left as placeholders for future extension.
    class DefaultSceneCaptureRenderer : public ScriptableCaptureRenderer
    {
    public:
        DefaultSceneCaptureRenderer();
        ~DefaultSceneCaptureRenderer() override;

        void Render(RenderGraph& graph, const RenderCaptureContext& ctx) override;

    private:
        PerPassResources m_perPassResources;
    };

} // namespace pulsar
