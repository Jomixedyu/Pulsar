#pragma once

#include "Texture.h"
#include <Pulsar/IGPUResource.h>
#include <Pulsar/Assets/Texture.h>
#include <gfx/GFXFrameBufferObject.h>
#include <gfx/GFXRenderTarget.h>

namespace pulsar
{
    class Application;

    using RenderTargetType = gfx::GFXRenderTargetType;

    class RenderTexture : public Texture
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::RenderTexture, Texture);
        CORELIB_CLASS_ATTR(new MenuItemCreateAssetAttribute);
    public:
        RenderTexture();
        ~RenderTexture();
    public:
        //Texture
        virtual int32_t GetWidth() const override { return m_width; }
        virtual int32_t GetHeight() const override { return m_height; }
    public:
        
        void PostInitializeData(int32_t width, int32_t height);

        void Initialize(int32_t width, int32_t height, TextureFormat format, bool enableDepthStencil);

        static ObjectPtr<RenderTexture> StaticCreate(index_string name, int width, int height, bool hasColor, bool hasDepth);
    public:
        //IGPUResource
        virtual void CreateGPUResource() override;
        virtual void DestroyGPUResource() override;
        virtual bool IsCreatedGPUResource() const override;
        void EnableRenderTarget();
        void DisableRenderTarget();

        const std::shared_ptr<gfx::GFXRenderTarget>& GetGfxRenderTarget0() const { return m_color0; }
        const std::shared_ptr<gfx::GFXFrameBufferObject>& GetGfxFrameBufferObject() const { return m_framebuffer; }

    protected:
        //int32_t width_;
        //int32_t height_;

        //uint32_t buffer_;
        //int32_t last_buffer_;

        int32_t m_width;
        int32_t m_height;

        std::shared_ptr<gfx::GFXRenderTarget> m_color0;
        std::shared_ptr<gfx::GFXRenderTarget> m_depth;
        std::shared_ptr<gfx::GFXFrameBufferObject> m_framebuffer;
    };
    DECL_PTR(RenderTexture);

}