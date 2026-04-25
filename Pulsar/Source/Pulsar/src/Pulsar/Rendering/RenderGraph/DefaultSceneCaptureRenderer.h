#pragma once
#include "ScriptableCaptureRenderer.h"
#include "../PerPassResources.h"
#include <Pulsar/Assets/Material.h>
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
        void EnsureTonemapMaterial();
        void EnsureGammaMaterial();

    private:
        PerPassResources m_perPassResources;

        // Built-in post-process materials
        RCPtr<Material> m_tonemapMaterial;
        RCPtr<Material> m_gammaMaterial;

        gfx::GFXDescriptorSetLayout_sp m_ppRendererLayout;
        gfx::GFXDescriptorSet_sp m_ppTonemapSet;
        gfx::GFXDescriptorSet_sp m_ppGammaSet;
        gfx::GFXDescriptorSet_sp m_ppCustomSet;
    };

} // namespace pulsar
