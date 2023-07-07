#pragma once

#include <Pulsar/Assets/Texture.h>
#include <Pulsar/IBindGPU.h>
#include "Texture.h"


namespace pulsar
{
    class RenderTexture : public Texture
    {
        CORELIB_DEF_TYPE(AssemblyObject_Pulsar, pulsar::RenderTexture, Texture);

    public:
        RenderTexture();
        ~RenderTexture();
    public:
        //Texture
        virtual int32_t get_width() const override;
        virtual int32_t get_height() const override;
    public:
        void PostInitializeData(int32_t width, int32_t height);
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