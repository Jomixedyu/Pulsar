#pragma once

#include <Pulsar/Assets/Texture.h>
#include <Pulsar/IBindGPU.h>
#include "Texture.h"
#include <gfx/GFXRenderTarget.h>
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
        virtual int32_t GetWidth() const override;
        virtual int32_t GetHeight() const override;
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

    protected:
        int32_t width_;
        int32_t height_;

        uint32_t buffer_;
        int32_t last_buffer_;

        std::shared_ptr<gfx::GFXRenderTarget> m_color0;
        std::shared_ptr<gfx::GFXRenderTarget> m_depth;
    };
    CORELIB_DECL_SHORTSPTR(RenderTexture);

    struct RenderTextureScope
    {
        RenderTexture_sp tex_;
        Vector2i last_size_;
    public:
        RenderTextureScope(RenderTexture_sp tex);
        ~RenderTextureScope();
    };
}