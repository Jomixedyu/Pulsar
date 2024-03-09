#include "Assets/RenderTexture.h"
#include <Pulsar/Application.h>

namespace pulsar
{
    RenderTexture::RenderTexture()
    {
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

    RCPtr<RenderTexture> RenderTexture::StaticCreate(index_string name, int width, int height, int colorRTCount, bool hasDepthStencil)
    {
        auto self = mksptr(new RenderTexture);
        self->Construct();
        self->SetIndexName(name);

        auto gfx = Application::GetGfxApp();

        auto depthFormats = gfx->GetSupportedDepthFormats();
        assert(depthFormats.size() != 0);

        for (int i = 0; i < colorRTCount; ++i)
        {
            auto colorRt = gfx->CreateRenderTarget(width, height, gfx::GFXRenderTargetType::Color, gfx::GFXTextureFormat::R8G8B8A8_UNorm, {});
            self->m_renderTargets.push_back(colorRt);
        }

        if (hasDepthStencil)
        {
            auto depth = gfx->CreateRenderTarget(width, height, gfx::GFXRenderTargetType::DepthStencil, depthFormats[0], {});;
            self->m_renderTargets.push_back(depth);
        }

        std::vector<gfx::GFXRenderTarget*> rts;
        for (auto rt : self->m_renderTargets)
        {
            rts.push_back(rt.get());
        }

        auto renderPass = gfx->CreateRenderPassLayout(rts);
        self->m_framebuffer = gfx->CreateFrameBufferObject(rts, renderPass);

        self->m_width = width;
        self->m_height = height;

        return self.get();
    }

    void RenderTexture::PostInitializeData(int32_t width, int32_t height)
    {
    }

} // namespace pulsar