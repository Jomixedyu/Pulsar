#include <Pulsar/Rendering/RenderTextureProxy.h>

#include <Pulsar/Assets/RenderTexture.h>
#include <Pulsar/Application.h>
#include <gfx/GFXResourceRegistry.h>

namespace pulsar::rendering
{
    void RenderTextureProxy::Init(int32_t width, int32_t height, uint32_t sampleCount,
                                  RenderTextureColorFormat colorFormat,
                                  RenderTextureDepthFormat depthFormat)
    {
        Release();

        auto gfxApp = Application::GetGfxApp();
        auto* registry = gfxApp ? gfxApp->GetResourceRegistry() : nullptr;
        if (!registry)
            return;

        gfx::GFXSamplerConfig samplerCfg{};
        samplerCfg.Filter = gfx::GFXSamplerFilter::Linear;
        samplerCfg.AddressMode = gfx::GFXSamplerAddressMode::ClampToEdge;

        m_colorTexture = registry->CreateRenderTarget(
            width, height, gfx::GFXTextureTargetType::ColorTarget,
            RenderTexture::ToGFXFormat(colorFormat), samplerCfg, sampleCount, false);

        if (depthFormat != RenderTextureDepthFormat::None)
        {
            bool isDepthStencil = (depthFormat == RenderTextureDepthFormat::D32_SFloat_S8_UInt ||
                                   depthFormat == RenderTextureDepthFormat::D24_UNorm_S8_UInt);

            m_depthTexture = registry->CreateRenderTarget(
                width, height,
                isDepthStencil ? gfx::GFXTextureTargetType::DepthStencilTarget
                               : gfx::GFXTextureTargetType::DepthTarget,
                RenderTexture::ToGFXFormat(depthFormat), samplerCfg, sampleCount, false);
        }

        std::vector<gfx::GFXTexture2DViewPtr> views;
        views.reserve(m_depthTexture ? 2 : 1);
        if (m_colorTexture)
            views.push_back(m_colorTexture->Get2DView(0));
        if (m_depthTexture)
            views.push_back(m_depthTexture->Get2DView(0));

        m_framebuffer = registry->CreateFrameBufferObject(views);
        m_initialized = true;
    }

    void RenderTextureProxy::Release()
    {
        // Member resets only; the registry defers the actual GPU destruction.
        // Safe from any thread (also runs from the destructor).
        m_framebuffer.reset();
        m_depthTexture.reset();
        m_colorTexture.reset();
        m_initialized = false;
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