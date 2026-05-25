#pragma once

#include "Texture2DBase.h"
#include "RenderTextureFormats.h"
#include <Pulsar/IGPUResource.h>
#include <Pulsar/Rendering/RenderProxyTexture.h>
#include <gfx/GFXFrameBufferObject.h>
#include <gfx/GFXTexture.h>

namespace pulsar
{
    class Application;

    using RenderTargetType = gfx::GFXTextureTargetType;

    class RenderTexture : public Texture2DBase
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::RenderTexture, Texture2DBase);
        CORELIB_CLASS_ATTR(new CreateAssetAttribute);
    public:
        RenderTexture();
        ~RenderTexture() override;
        void OnDestroy() override;

        void Serialize(AssetSerializer* s) override;

        // Texture
        int32_t GetWidth() const override { return m_width; }
        int32_t GetHeight() const override { return m_height; }
        std::shared_ptr<gfx::GFXTexture> GetGFXTexture() const override;

        void PostEditChange(FieldInfo* info) override;

        void PostInitializeData(int32_t width, int32_t height);
        static array_list<gfx::GFXTextureFormat> GetSupportedDepthFormats();
        static bool IsSupportedDepthFormat(gfx::GFXTextureFormat format);
        static gfx::GFXTextureFormat ToGFXFormat(RenderTextureColorFormat format);
        static gfx::GFXTextureFormat ToGFXFormat(RenderTextureDepthFormat format);
        static std::optional<RenderTextureColorFormat> FromGFXColorFormat(gfx::GFXTextureFormat format);
        static std::optional<RenderTextureDepthFormat> FromGFXDepthFormat(gfx::GFXTextureFormat format);

        // IGPUResource
        bool CreateGPUResource() override;
        void DestroyGPUResource() override;
        bool IsCreatedGPUResource() const override;
        void EnableRenderTarget();
        void DisableRenderTarget();

        std::shared_ptr<gfx::GFXTexture2DView> GetGfxRenderTarget0() const;
        std::shared_ptr<gfx::GFXFrameBufferObject> GetGfxFrameBufferObject() const;
        const array_list<gfx::GFXTexture_sp>& GetRenderTargets() const;

        List_sp<RenderTextureColorFormat> GetColorFormats() const { return m_colorFormats; }
        uint32_t GetSampleCount() const { return m_sampleCount; }
        RenderTextureDepthFormat GetDepthFormat() const { return m_depthFormat; }

    protected:
        void RebuildGPUResources();

        SPtr<RenderProxyRenderTexture> m_proxy;

        bool m_createdGPUResource = false;

    public:
        CORELIB_REFL_DECL_FIELD(m_width, new RangePropertyAttribute(1, 8192))
        int32_t m_width = 512;

        CORELIB_REFL_DECL_FIELD(m_height, new RangePropertyAttribute(1, 8192))
        int32_t m_height = 512;

        CORELIB_REFL_DECL_FIELD(m_sampleCount)
        uint32_t m_sampleCount = 1;

        CORELIB_REFL_DECL_FIELD(m_colorFormats)
        List_sp<RenderTextureColorFormat> m_colorFormats;

        CORELIB_REFL_DECL_FIELD(m_depthFormat)
        RenderTextureDepthFormat m_depthFormat = RenderTextureDepthFormat::D32_SFloat;
    };
    DECL_PTR(RenderTexture);

} // namespace pulsar
