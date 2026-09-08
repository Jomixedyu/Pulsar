#include <Pulsar/Rendering/RenderTextureProxy.h>

#include <Pulsar/Assets/RenderTexture.h>
#include <Pulsar/Application.h>
#include <gfx/GFXResourceManager.h>

namespace pulsar::rendering
{
    RenderTextureProxy::RenderTextureProxy(int32_t width, int32_t height, uint32_t sampleCount,
                                           RenderTextureColorFormat colorFormat,
                                           RenderTextureDepthFormat depthFormat)
        : m_width(width)
        , m_height(height)
        , m_sampleCount(sampleCount)
        , m_colorFormat(colorFormat)
        , m_depthFormat(depthFormat)
    {
    }

    void RenderTextureProxy::OnCreateResource()
    {
        if (m_created)
            return;

        CreateResources();
    }

    void RenderTextureProxy::OnDestroyResource()
    {
        DestroyResources();
    }

    void RenderTextureProxy::CreateResources()
    {
        auto gfxApp = Application::GetGfxApp();
        if (!gfxApp)
            return;

        auto* resMgr = gfxApp->GetResourceManager();
        gfx::GFXSamplerConfig samplerCfg{};
        samplerCfg.Filter = gfx::GFXSamplerFilter::Linear;
        samplerCfg.AddressMode = gfx::GFXSamplerAddressMode::ClampToEdge;

        {
            gfx::GFXTextureCreateDesc desc{};
            desc.Width = m_width;
            desc.Height = m_height;
            desc.TargetType = gfx::GFXTextureTargetType::ColorTarget;
            desc.Format = RenderTexture::ToGFXFormat(m_colorFormat);
            desc.SamplerCfg = samplerCfg;
            desc.SampleCount = m_sampleCount;
            desc.IsTransientAttachment = false;

            m_colorHandle = resMgr->AllocHandle<gfx::TextureHandle>();
            resMgr->CreateRenderTarget(m_colorHandle, desc);
        }

        if (m_depthFormat != RenderTextureDepthFormat::None)
        {
            bool isDepthStencil = (m_depthFormat == RenderTextureDepthFormat::D32_SFloat_S8_UInt ||
                                   m_depthFormat == RenderTextureDepthFormat::D24_UNorm_S8_UInt);

            gfx::GFXTextureCreateDesc desc{};
            desc.Width = m_width;
            desc.Height = m_height;
            desc.TargetType = isDepthStencil ? gfx::GFXTextureTargetType::DepthStencilTarget
                                             : gfx::GFXTextureTargetType::DepthTarget;
            desc.Format = RenderTexture::ToGFXFormat(m_depthFormat);
            desc.SamplerCfg = samplerCfg;
            desc.SampleCount = m_sampleCount;
            desc.IsTransientAttachment = false;

            m_depthHandle = resMgr->AllocHandle<gfx::TextureHandle>();
            resMgr->CreateRenderTarget(m_depthHandle, desc);
        }

        std::vector<gfx::GFXTexture2DView_sp> views;
        views.reserve(m_depthHandle.IsValid() ? 2 : 1);

        if (auto color = resMgr->GetTextureShared(m_colorHandle))
            views.push_back(color->Get2DView(0));

        if (auto depth = resMgr->GetTextureShared(m_depthHandle))
            views.push_back(depth->Get2DView(0));

        m_framebufferHandle = resMgr->AllocHandle<gfx::FrameBufferObjectHandle>();
        resMgr->CreateFrameBufferObject(m_framebufferHandle, views);
        m_framebuffer = resMgr->GetFrameBufferObjectShared(m_framebufferHandle);
        m_created = true;
    }

    void RenderTextureProxy::DestroyResources()
    {
        if (!m_created)
            return;

        if (auto gfxApp = Application::GetGfxApp())
        {
            auto* resMgr = gfxApp->GetResourceManager();
            if (m_framebufferHandle.IsValid())
                resMgr->Destroy(m_framebufferHandle);

            if (m_depthHandle.IsValid())
                resMgr->Destroy(m_depthHandle);

            if (m_colorHandle.IsValid())
                resMgr->Destroy(m_colorHandle);
        }

        m_framebufferHandle = {};
        m_framebuffer.reset();
        m_depthHandle = {};
        m_colorHandle = {};
        m_created = false;
    }

    gfx::TextureHandle RenderTextureProxy::GetTextureHandle() const
    {
        return m_colorHandle;
    }

    gfx::TextureHandle RenderTextureProxy::GetDepthTextureHandle() const
    {
        return m_depthHandle;
    }

    gfx::GFXTexture2DView_sp RenderTextureProxy::GetColorTextureView() const
    {
        if (!m_colorHandle.IsValid())
            return nullptr;

        auto gfxApp = Application::GetGfxApp();
        if (!gfxApp)
            return nullptr;

        auto texture = gfxApp->GetResourceManager()->GetTextureShared(m_colorHandle);
        return texture ? texture->Get2DView(0) : nullptr;
    }

    gfx::GFXTexture2DView_sp RenderTextureProxy::GetDepthRenderTarget() const
    {
        if (!m_depthHandle.IsValid())
            return nullptr;

        auto gfxApp = Application::GetGfxApp();
        if (!gfxApp)
            return nullptr;

        auto texture = gfxApp->GetResourceManager()->GetTextureShared(m_depthHandle);
        return texture ? texture->Get2DView(0) : nullptr;
    }

    gfx::GFXFrameBufferObject_sp RenderTextureProxy::GetFrameBufferObject() const
    {
        return m_framebuffer;
    }

    array_list<gfx::GFXTexture_sp> RenderTextureProxy::GetFramebufferAttachments() const
    {
        array_list<gfx::GFXTexture_sp> result;
        auto gfxApp = Application::GetGfxApp();
        if (!gfxApp)
            return result;

        auto* resMgr = gfxApp->GetResourceManager();
        result.reserve(m_depthHandle.IsValid() ? 2 : 1);

        if (auto color = resMgr->GetTextureShared(m_colorHandle))
            result.push_back(color);

        if (auto depth = resMgr->GetTextureShared(m_depthHandle))
            result.push_back(depth);

        return result;
    }
}
