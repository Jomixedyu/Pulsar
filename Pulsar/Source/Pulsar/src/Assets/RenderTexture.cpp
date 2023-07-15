#include "Assets/RenderTexture.h"
#include <ThirdParty/glad/glad.h>
#include <Pulsar/Private/RenderInterface.h>

namespace pulsar
{
    RenderTexture::RenderTexture()
    {

    }
    void RenderTexture::Initialize(int32_t width, int32_t height, TextureFormat format, bool enableDepthStencil)
    {
        m_color0 = Application::GetGfxApp()->CreateRenderTarget(width, height, RenderTargetType::Color, format, {});
        if (enableDepthStencil)
        {
            m_depth = Application::GetGfxApp()->CreateRenderTarget(width, height, RenderTargetType::Depth, format, {});
        }
    }

    RenderTexture::~RenderTexture()
    {
        this->UnBindGPU();
    }
    void RenderTexture::BindGPU()
    {
        //assert(!this->GetIsBindGPU());
        //glGenFramebuffers(1, &this->buffer_);
        //glBindFramebuffer(GL_FRAMEBUFFER, this->buffer_);

        //glGenTextures(1, &this->tex_id_);
        //glBindTexture(GL_TEXTURE_2D, this->tex_id_);

        //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, this->width_, this->height_, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        //glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->tex_id_, 0);

        //glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    void RenderTexture::UnBindGPU()
    {
        if (this->GetIsBindGPU())
        {


            this->tex_id_ = 0;
        }
    }
    bool RenderTexture::GetIsBindGPU()
    {
        return this->tex_id_ != 0;
    }

    void RenderTexture::EnableRenderTarget()
    {
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &this->last_buffer_);
        glBindFramebuffer(GL_FRAMEBUFFER, this->buffer_);
        detail::RenderInterface::SetViewport(0, 0, this->width_, this->height_);
    }

    void RenderTexture::DisableRenderTarget()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, this->last_buffer_);
    }

    int32_t RenderTexture::GetWidth() const
    {
        return this->width_;
    }
    int32_t RenderTexture::GetHeight() const
    {
        return this->height_;
    }
    void RenderTexture::PostInitializeData(int32_t width, int32_t height)
    {
        this->width_ = width;
        this->height_ = height;

    }


    RenderTextureScope::RenderTextureScope(RenderTexture_sp tex)
        : tex_(tex)
    {
        tex->EnableRenderTarget();
    }

    RenderTextureScope::~RenderTextureScope()
    {
        this->tex_->DisableRenderTarget();
    }
}