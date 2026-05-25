#include "RenderProxyTexture.h"

#include <Pulsar/Assets/Texture2D.h>
#include <Pulsar/Assets/TextureCube.h>
#include <Pulsar/Assets/RenderTexture.h>
#include <Pulsar/Assets/RenderTextureCube.h>
#include <Pulsar/Application.h>
#include <Pulsar/Util/TextureCompressionUtil.h>
#include <gfx/GFXImage.h>

namespace pulsar
{
    static gfx::GFXTextureFormat _GetTextureFormat(TextureCompressionFormat format)
    {
        return Texture2D::StaticGetFormatMapping(OSPlatform::Windows64)->at(format);
    }

    // -----------------------------------------------------------------------
    // RenderProxyTexture2D
    // -----------------------------------------------------------------------
    RenderProxyTexture2D::RenderProxyTexture2D(Texture2D* source)
        : m_source(source)
    {
    }

    void RenderProxyTexture2D::SetSource(Texture2D* source)
    {
        m_source = source;
        ReleaseRHI();
    }

    void RenderProxyTexture2D::InitRHI()
    {
        if (!m_source || m_tex)
            return;

        auto targetGfxFormat = _GetTextureFormat(m_source->GetCompressedFormat());

        std::vector<uint8_t> data{};
#ifdef WITH_EDITOR
        {
            std::vector<uint8_t> uncompressedData;
            if (m_source->IsOriginMemoryCompressed())
            {
                uncompressedData = gfx::LoadImageFromMemory(
                    m_source->GetOriginMemoryData(), m_source->GetOriginCompressedBinarySize(),
                    nullptr, nullptr, nullptr, m_source->GetChannelCount());
            }
            else
            {
                uncompressedData.assign(
                    m_source->GetOriginMemoryData(),
                    m_source->GetOriginMemoryData() + m_source->GetOriginCompressedBinarySize());
            }

            data = TextureCompressionUtil::Compress(
                std::move(uncompressedData),
                m_source->GetWidth(),
                m_source->GetHeight(),
                m_source->GetChannelCount(),
                targetGfxFormat);
        }
#else
        // TODO: runtime path - load from pre-baked native data
#endif

        SamplerConfig samplerConfig;
        samplerConfig.Filter = m_source->GetSamplerFilter();
        samplerConfig.AddressMode = m_source->GetSamplerAddressMode();

        m_tex = Application::GetGfxApp()->CreateTexture2DFromMemory(
            data.data(), data.size(),
            m_source->GetWidth(), m_source->GetHeight(),
            targetGfxFormat,
            samplerConfig);
    }

    void RenderProxyTexture2D::ReleaseRHI()
    {
        m_tex.reset();
    }

    // -----------------------------------------------------------------------
    // RenderProxyTextureCube
    // -----------------------------------------------------------------------
    RenderProxyTextureCube::RenderProxyTextureCube(TextureCube* source)
        : m_source(source)
    {
    }

    void RenderProxyTextureCube::SetSource(TextureCube* source)
    {
        m_source = source;
        ReleaseRHI();
    }

    void RenderProxyTextureCube::InitRHI()
    {
        if (!m_source || m_tex)
            return;

        auto gfxapp = Application::GetGfxApp();
        m_tex = gfxapp->CreateTextureCube(m_source->GetWidth());

        // TODO: move render-to-cube logic here if needed
    }

    void RenderProxyTextureCube::ReleaseRHI()
    {
        m_tex.reset();
    }

    // -----------------------------------------------------------------------
    // RenderProxyRenderTexture
    // -----------------------------------------------------------------------
    RenderProxyRenderTexture::RenderProxyRenderTexture(RenderTexture* source)
        : m_source(source)
    {
    }

    void RenderProxyRenderTexture::SetSource(RenderTexture* source)
    {
        m_source = source;
        ReleaseRHI();
    }

    void RenderProxyRenderTexture::InitRHI()
    {
        if (!m_source || m_framebuffer)
            return;

        auto gfx = Application::GetGfxApp();
        if (!gfx)
            return;

        auto colorFormats = m_source->GetColorFormats();
        if (colorFormats->empty())
        {
            colorFormats->push_back(RenderTextureColorFormat::RGBA8_UNorm);
        }

        gfx::GFXSamplerConfig samplerCfg{};
        samplerCfg.Filter = gfx::GFXSamplerFilter::Linear;
        samplerCfg.AddressMode = gfx::GFXSamplerAddressMode::ClampToEdge;

        for (auto& fmt : *colorFormats)
        {
            auto rt = gfx->CreateRenderTarget(m_source->GetWidth(), m_source->GetHeight(),
                gfx::GFXTextureTargetType::ColorTarget, RenderTexture::ToGFXFormat(fmt),
                samplerCfg, m_source->GetSampleCount(), false);
            m_renderTargets.push_back(rt);
        }

        if (m_source->GetDepthFormat() != RenderTextureDepthFormat::None)
        {
            auto depthFmt = RenderTexture::ToGFXFormat(m_source->GetDepthFormat());
            bool isDepthStencil = (m_source->GetDepthFormat() == RenderTextureDepthFormat::D32_SFloat_S8_UInt
                || m_source->GetDepthFormat() == RenderTextureDepthFormat::D24_UNorm_S8_UInt);
            auto targetType = isDepthStencil ? gfx::GFXTextureTargetType::DepthStencilTarget : gfx::GFXTextureTargetType::DepthTarget;
            auto rt = gfx->CreateRenderTarget(m_source->GetWidth(), m_source->GetHeight(),
                targetType, depthFmt, samplerCfg, m_source->GetSampleCount(), false);
            m_renderTargets.push_back(rt);
        }

        std::vector<gfx::GFXTexture2DView_sp> views;
        for (auto& rt : m_renderTargets)
        {
            views.push_back(rt->Get2DView(0));
        }
        m_framebuffer = gfx->CreateFrameBufferObject(views);
    }

    void RenderProxyRenderTexture::ReleaseRHI()
    {
        m_framebuffer.reset();
        m_renderTargets.clear();
    }

    // -----------------------------------------------------------------------
    // RenderProxyRenderTextureCube
    // -----------------------------------------------------------------------
    RenderProxyRenderTextureCube::RenderProxyRenderTextureCube(RenderTextureCube* source)
        : m_source(source)
    {
    }

    void RenderProxyRenderTextureCube::SetSource(RenderTextureCube* source)
    {
        m_source = source;
        ReleaseRHI();
    }

    void RenderProxyRenderTextureCube::InitRHI()
    {
        if (!m_source || m_tex)
            return;

        auto gfxapp = Application::GetGfxApp();
        m_tex = gfxapp->CreateTextureCube(m_source->GetWidth());
    }

    void RenderProxyRenderTextureCube::ReleaseRHI()
    {
        m_tex.reset();
    }

} // namespace pulsar
