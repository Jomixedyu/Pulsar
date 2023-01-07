#pragma once

#include <Apatite/Assets/Texture.h>
#include <Apatite/IBindGPU.h>
#include "Texture.h"


namespace apatite
{
    class RenderTexture : public Texture
    {
        CORELIB_DEF_TYPE(AssemblyObject_Apatite, apatite::RenderTexture, Texture);

    public:
        RenderTexture();
        ~RenderTexture();
    public:
        // 通过 Texture 继承
        virtual int32_t get_width() const override;
        virtual int32_t get_height() const override;
    public:
        void PostInitialize(int32_t width, int32_t height);
    public:
        // 通过 IBindGPU 继承
        virtual void BindGPU() override;
        virtual void UnBindGPU() override;
        virtual bool GetIsBindGPU() override;
        void EnableRenderTarget();
        void DisableRenderTarget();

    protected:
        int32_t width_;
        int32_t height_;
        uint32_t buffer_;
    };
    CORELIB_DECL_SHORTSPTR(RenderTexture);

    struct RenderTextureRAII
    {
        RenderTexture_sp tex_;
    public:
        RenderTextureRAII(RenderTexture_sp tex): tex_(tex)
        { 
            tex->EnableRenderTarget(); 
        }
        ~RenderTextureRAII() { this->tex_->DisableRenderTarget(); }
    };
}