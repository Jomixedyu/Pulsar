#include "Assets/RenderTexture.h"
#include <Pulsar/Application.h>

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
        this->DestroyGPUResource();
    }
    void RenderTexture::CreateGPUResource()
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
    void RenderTexture::DestroyGPUResource()
    {
        if (this->IsCreatedGPUResource())
        {


            this->tex_id_ = 0;
        }
    }
    bool RenderTexture::IsCreatedGPUResource() const
    {
        return this->tex_id_ != 0;
    }

    void RenderTexture::EnableRenderTarget()
    {

    }

    void RenderTexture::DisableRenderTarget()
    {

    }

    RenderTexture_ref RenderTexture::StaticCreate(index_string name, int width, int height, bool hasColor, bool hasDepth)
    {
        auto self = mksptr(new RenderTexture);
        self->Construct();
        self->m_name = name;

        auto gfx = Application::GetGfxApp();

        auto depthFormats = gfx->GetSupportedDepthFormats();
        assert(depthFormats.size() != 0);

        
        self->m_color0 = gfx->CreateRenderTarget(width, height, gfx::GFXRenderTargetType::Color, gfx::GFXTextureFormat::R8G8B8A8_SRGB, {});
        self->m_depth = gfx->CreateRenderTarget(width, height, gfx::GFXRenderTargetType::DepthStencil, depthFormats[0], {});


        std::vector rts = { self->m_color0.get(), self->m_depth.get() };

        auto renderPass = gfx->CreateRenderPassLayout(rts);
        self->m_framebuffer = gfx->CreateFrameBufferObject(rts, renderPass);

        self->m_width = width;
        self->m_height = height;

        return self;
    }


    void RenderTexture::PostInitializeData(int32_t width, int32_t height)
    {


    }


}