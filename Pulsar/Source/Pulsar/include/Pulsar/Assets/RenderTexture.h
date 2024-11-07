#pragma once

#include "Texture.h"
#include <Pulsar/IGPUResource.h>
#include <Pulsar/Assets/Texture.h>
#include <gfx/GFXFrameBufferObject.h>
#include <gfx/GFXTexture.h>

namespace pulsar
{
    class Application;

    using RenderTargetType = gfx::GFXTextureTargetType;

    struct RenderTargetInfo
    {
        gfx::GFXTextureTargetType TargetType;
        gfx::GFXTextureFormat Format;
    };

    class RenderTexture : public Texture
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::RenderTexture, Texture);
        CORELIB_CLASS_ATTR(new CreateAssetAttribute);
    public:
        RenderTexture();
        ~RenderTexture() override;
        void OnDestroy() override;
    public:
        //Texture
        int32_t GetWidth() const override { return m_width; }
        int32_t GetHeight() const override { return m_height; }
    public:
        
        void PostInitializeData(int32_t width, int32_t height);
        static array_list<gfx::GFXTextureFormat> GetSupportedDepthFormats();
        static bool IsSupportedDepthFormat(gfx::GFXTextureFormat format);

        static RCPtr<RenderTexture> StaticCreate(index_string name, int width, int height,
            const array_list<RenderTargetInfo>& targetInfo);
    public:
        //IGPUResource
        bool CreateGPUResource() override;
        void DestroyGPUResource() override;
        bool IsCreatedGPUResource() const override;
        void EnableRenderTarget();
        void DisableRenderTarget();

        std::shared_ptr<gfx::GFXTexture2DView> GetGfxRenderTarget0() const { return m_renderTargets[0]->Get2DView(0); }
        const std::shared_ptr<gfx::GFXFrameBufferObject>& GetGfxFrameBufferObject() const { return m_framebuffer; }
        const array_list<gfx::GFXTexture_sp>& GetRenderTargets() const { return m_renderTargets; }

    protected:

        int32_t m_width;
        int32_t m_height;

        array_list<gfx::GFXTexture_sp> m_renderTargets;
        std::shared_ptr<gfx::GFXFrameBufferObject> m_framebuffer;

        bool m_createdGPUResource = false;
    };
    DECL_PTR(RenderTexture);

}