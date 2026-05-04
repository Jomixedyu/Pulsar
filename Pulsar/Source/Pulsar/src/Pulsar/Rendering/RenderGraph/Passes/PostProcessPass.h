#pragma once
#include "RenderFeature.h"
#include <Pulsar/Assets/Material.h>
#include <gfx/GFXDescriptorSet.h>

namespace pulsar
{
    class CameraComponent;
    class PerPassResources;

    class PostProcessPass : public RenderFeature
    {
    public:
        explicit PostProcessPass(RCPtr<Material> material);
        virtual ~PostProcessPass() = default;

        void Initialize(PerPassResources* perPass);
        void Destroy();

        RGTextureHandle AddToGraph(RenderGraph& graph,
                                   RGTextureHandle hSrc,
                                   RGTextureHandle hDst,
                                   CameraComponent* cam,
                                   PerPassResources* perPass) override;

        void SetMaterial(RCPtr<Material> material) { m_material = material; }

    public:
        bool IsPassEnabled() const { return IsEnabled(); }

    protected:
        virtual void PrepareMaterial(CameraComponent* cam) {}
        virtual bool IsEnabled() const { return m_material != nullptr; }
        virtual const char* GetPassName() const = 0;

        void DrawFullscreen(RGPassContext& passCtx, gfx::GFXCommandBuffer& cmdBuffer,
                            RGTextureHandle hSrc, RGTextureHandle hDst,
                            CameraComponent* cam, PerPassResources* perPass);

        gfx::GFXDescriptorSetLayout_sp GetInputSamplerLayout();

        RCPtr<Material> m_material;
        gfx::GFXDescriptorSet_sp m_descriptorSet;
        gfx::GFXDescriptorSet_sp m_perPassSet;
        gfx::GFXDescriptorSetLayout_sp m_inputSamplerLayout;
    };
}
