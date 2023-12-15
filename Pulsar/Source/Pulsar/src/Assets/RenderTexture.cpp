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
    }
    void RenderTexture::OnDestroy()
    {
        base::OnDestroy();
        DestroyGPUResource();
    }
    bool RenderTexture::CreateGPUResource()
    {
        if (IsCreatedGPUResource())
        {
            return true;
        }
        m_createdGPUResource = true;
        return true;
    }
    void RenderTexture::DestroyGPUResource()
    {
        if (!IsCreatedGPUResource())
        {
            return;
        }
        m_createdGPUResource = false;
    }
    bool RenderTexture::IsCreatedGPUResource() const
    {
        return m_createdGPUResource;
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
        self->SetIndexName(name);

        auto gfx = Application::GetGfxApp();

        auto depthFormats = gfx->GetSupportedDepthFormats();
        assert(depthFormats.size() != 0);

        self->m_color0 = gfx->CreateRenderTarget(width, height, gfx::GFXRenderTargetType::Color, gfx::GFXTextureFormat::R8G8B8A8_SRGB, {});
        self->m_depth = gfx->CreateRenderTarget(width, height, gfx::GFXRenderTargetType::DepthStencil, depthFormats[0], {});

        std::vector rts = {self->m_color0.get(), self->m_depth.get()};

        auto renderPass = gfx->CreateRenderPassLayout(rts);
        self->m_framebuffer = gfx->CreateFrameBufferObject(rts, renderPass);

        self->m_width = width;
        self->m_height = height;

        return self;
    }

    void RenderTexture::PostInitializeData(int32_t width, int32_t height)
    {
    }

} // namespace pulsar