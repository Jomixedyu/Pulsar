#pragma once
#include "RenderFeature.h"
#include <Pulsar/Assets/Material.h>
#include <gfx/GFXDescriptorSet.h>
#include <gfx/GFXFrameBufferObject.h>
#include <gfx/GFXTexture.h>

namespace pulsar
{
    class PostProcessPass : public RenderFeature
    {
    public:
        explicit PostProcessPass(RCPtr<Material> material);
        virtual ~PostProcessPass() = default;

        void Initialize();
        void Destroy();

        RGTextureHandle AddToGraph(RenderGraph& graph,
                                   RGTextureHandle hSrc,
                                   RGTextureHandle hDst,
                                   const RenderCaptureContext& ctx) override;

        void SetMaterial(RCPtr<Material> material) { m_material = material; }

    public:
        bool IsPassEnabled() const { return IsEnabled(); }

    protected:
        virtual void PrepareMaterial(const RenderCaptureContext& ctx) {}
        virtual bool IsEnabled() const { return m_material != nullptr; }
        virtual const char* GetPassName() const = 0;

        void DrawFullscreen(RGPassContext& passCtx, gfx::GFXCommandBuffer& cmdBuffer,
                            RGTextureHandle hSrc, RGTextureHandle hDst,
                            const gfx::GFXFrameBufferObject_sp& fallbackFBO,
                            const gfx::GFXTexture2DView_sp& fallbackView,
                            gfx::GFXBuffer* cameraBuffer,
                            gfx::GFXBuffer* worldBuffer);

        gfx::GFXDescriptorSetLayout_sp GetInputSamplerLayout();

        RCPtr<Material> m_material;
        // Render-side mirror resolved from m_material in AddToGraph (game thread).
        // Render-thread lambdas consume this instead of the RCPtr<Material>.
        std::shared_ptr<MaterialProxy> m_proxy;
        gfx::GFXDescriptorSet_sp m_descriptorSet;
        gfx::GFXDescriptorSetLayout_sp m_inputSamplerLayout;
    };
}
