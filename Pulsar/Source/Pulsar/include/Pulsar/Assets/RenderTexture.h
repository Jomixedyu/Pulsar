#pragma once

#include <Pulsar/Assets/Texture.h>
#include <Pulsar/IBindGPU.h>
#include "Texture.h"
#include <gfx/GFXRenderTarget.h>
#include <gfx/GFXFrameBufferObject.h>
#include <Pulsar/Application.h>
namespace pulsar
{
    using RenderTargetType = gfx::GFXRenderTargetType;

    class RenderTexture : public Texture
    {
        CORELIB_DEF_TYPE(AssemblyObject_Pulsar, pulsar::RenderTexture, Texture);

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

        static sptr<RenderTexture> StaticCreate(int width, int height, bool hasColor, bool hasDepth);
    public:
        //IBindGPU
        virtual void BindGPU() override;
        virtual void UnBindGPU() override;
        virtual bool GetIsBindGPU() override;
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