#pragma once
#include "ScriptableCaptureRenderer.h"
#include "../PerPassResources.h"
#include <gfx/GFXDescriptorSet.h>
#include <unordered_map>

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

        // Cached per-Renderer (set2) resources for PP_InColor binding.
        // One slot per post-process index (not keyed by material pointer to avoid
        // stale-pointer issues when materials are replaced).
        struct PPRendererResources
        {
            gfx::GFXDescriptorSetLayout_sp layout;
            gfx::GFXDescriptorSet_sp       set;
        };
        std::vector<PPRendererResources> m_ppRendererCache;
    };

} // namespace pulsar
