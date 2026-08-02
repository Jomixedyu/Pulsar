#pragma once
#include "RenderFeature.h"
#include <Pulsar/Assets/Material.h>
#include <gfx/GFXDescriptorSet.h>
#include <gfx/GFXBuffer.h>
#include <gfx/GFXHandle.h>

namespace pulsar
{
    class BloomPass : public RenderFeature
    {
    public:
        BloomPass();
        ~BloomPass() override;

        void Initialize();
        void Destroy();

        void ReadSettings(const VolumeStack& stack) override;

        RGTextureHandle AddToGraph(RenderGraph& graph,
                                   RGTextureHandle input,
                                   RGTextureHandle output,
                                   const RenderCaptureContext& ctx) override;

    public:
        bool IsEnabled() const override;

    private:
        void EnsureMaterial();
        void SetupBloomSet(gfx::GFXDescriptorSet* set, gfx::GFXTexture2DView* srcView);
        void SetupCombineSet(gfx::GFXDescriptorSet* set,
                             gfx::GFXTexture2DView* srcView,
                             gfx::GFXTexture2DView* v0,
                             gfx::GFXTexture2DView* v1,
                             gfx::GFXTexture2DView* v2,
                             gfx::GFXTexture2DView* v3);
        void WriteBloomParams(uint32_t idx, const Vector2f& texelSize, const Vector2f& direction, int32_t sampleMode, float threshold);

        struct BloomParams
        {
            Vector2f TexelSize;
            Vector2f Direction;
            float    Threshold;
            float    Intensity;
            int32_t  SampleMode;
            int32_t  _Padding0;
            int32_t  _Padding1;
            int32_t  _Padding2;
        };

        RCPtr<Material> m_material;
        // Render-side mirror resolved from m_material in AddToGraph (game thread).
        std::shared_ptr<MaterialProxy> m_proxy;

        bool  m_bloomEnabled = true;
        float m_bloomThreshold = 0.44922f;
        float m_bloomIntensity = 1.0f;

        gfx::GFXDescriptorSetLayout_sp m_bloomLayout;
        array_list<gfx::GFXDescriptorSet_sp> m_bloomSets;

        gfx::GFXDescriptorSetLayout_sp m_combineLayout;
        gfx::GFXDescriptorSet_sp       m_combineSet;

        array_list<gfx::BufferHandle> m_bloomParamBuffers;
    };
}
