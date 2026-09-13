#include <Pulsar/Rendering/RenderTextureProxy.h>

#include <Pulsar/Assets/RenderTexture.h>
#include <Pulsar/Application.h>
#include <gfx/GFXResourceRegistry.h>

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

        auto gfxApp = Application::GetGfxApp();
        auto* registry = gfxApp ? gfxApp->GetResourceRegistry() : nullptr;
        if (!registry)
            return;

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

            m_colorTexture = registry->CreateRenderTarget(
                m_width, m_height, gfx::GFXTextureTargetType::ColorTarget,
                desc.Format, samplerCfg, m_sampleCount, false);
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

            m_depthTexture = registry->CreateRenderTarget(
                m_width, m_height, desc.TargetType,
                desc.Format, samplerCfg, m_sampleCount, false);
        }

        std::vector<gfx::GFXTexture2DViewPtr> views;
        views.reserve(m_depthTexture ? 2 : 1);

        if (m_colorTexture)
            views.push_back(m_colorTexture->Get2DView(0));

        if (m_depthTexture)
            views.push_back(m_depthTexture->Get2DView(0));

        m_framebuffer = registry->CreateFrameBufferObject(views);
        m_created = true;
    }

    void RenderTextureProxy::OnDestroyResource()
    {
        if (!m_created)
            return;

        if (m_framebuffer)
        {
            m_framebuffer.reset();
        }

        m_depthTexture.reset();
        m_colorTexture.reset();
        m_created = false;
    }

    gfx::GFXTexture2DViewPtr RenderTextureProxy::GetColorTextureView() const
    {
        return m_colorTexture ? m_colorTexture->Get2DView(0) : nullptr;
    }

    gfx::GFXTexture2DViewPtr RenderTextureProxy::GetDepthRenderTarget() const
    {
        return m_depthTexture ? m_depthTexture->Get2DView(0) : nullptr;
    }

    gfx::GFXFrameBufferObjectPtr RenderTextureProxy::GetFrameBufferObject() const
    {
        return m_framebuffer;
    }

    array_list<gfx::GFXTexturePtr> RenderTextureProxy::GetFramebufferAttachments() const
    {
        array_list<gfx::GFXTexturePtr> result;
        result.reserve(m_depthTexture ? 2 : 1);

        if (m_colorTexture)
            result.push_back(m_colorTexture);

        if (m_depthTexture)
            result.push_back(m_depthTexture);

        return result;
    }
}
