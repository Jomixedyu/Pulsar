#include "Assets/RenderTexture.h"
#include <Pulsar/Application.h>
#include <optional>

namespace pulsar
{
    RenderTexture::RenderTexture()
    {
        init_sptr_member(m_colorFormats);
    }

    RenderTexture::~RenderTexture() = default;

    void RenderTexture::OnDestroy()
    {
        base::OnDestroy();
        DestroyGPUResource();
    }

    gfx::GFXTextureFormat RenderTexture::ToGFXFormat(RenderTextureColorFormat format)
    {
        switch (format)
        {
        case RenderTextureColorFormat::R8_UNorm:        return gfx::GFXTextureFormat::R8_UNorm;
        case RenderTextureColorFormat::R16_UNorm:       return gfx::GFXTextureFormat::R16_UNorm;
        case RenderTextureColorFormat::R32_SFloat:      return gfx::GFXTextureFormat::R32_SFloat;
        case RenderTextureColorFormat::RGBA8_UNorm:     return gfx::GFXTextureFormat::R8G8B8A8_UNorm;
        case RenderTextureColorFormat::RGBA8_SRGB:      return gfx::GFXTextureFormat::R8G8B8A8_SRGB;
        case RenderTextureColorFormat::BGRA8_UNorm:     return gfx::GFXTextureFormat::B8G8R8A8_UNorm;
        case RenderTextureColorFormat::RGBA16_SFloat:   return gfx::GFXTextureFormat::R16G16B16A16_SFloat;
        case RenderTextureColorFormat::RGBA32_SFloat:   return gfx::GFXTextureFormat::R32G32B32A32_SFloat;
        case RenderTextureColorFormat::R11G11B10_UFloat: return gfx::GFXTextureFormat::B10G11R11_UFloat;
        }
        return gfx::GFXTextureFormat::R8G8B8A8_UNorm;
    }

    gfx::GFXTextureFormat RenderTexture::ToGFXFormat(RenderTextureDepthFormat format)
    {
        switch (format)
        {
        case RenderTextureDepthFormat::D32_SFloat:          return gfx::GFXTextureFormat::D32_SFloat;
        case RenderTextureDepthFormat::D32_SFloat_S8_UInt:  return gfx::GFXTextureFormat::D32_SFloat_S8_UInt;
        case RenderTextureDepthFormat::D24_UNorm_S8_UInt:   return gfx::GFXTextureFormat::D24_UNorm_S8_UInt;
        case RenderTextureDepthFormat::None:
        default:
            return gfx::GFXTextureFormat::D32_SFloat;
        }
    }

    std::optional<RenderTextureColorFormat> RenderTexture::FromGFXColorFormat(gfx::GFXTextureFormat format)
    {
        switch (format)
        {
        case gfx::GFXTextureFormat::R8_UNorm:            return RenderTextureColorFormat::R8_UNorm;
        case gfx::GFXTextureFormat::R16_UNorm:           return RenderTextureColorFormat::R16_UNorm;
        case gfx::GFXTextureFormat::R32_SFloat:          return RenderTextureColorFormat::R32_SFloat;
        case gfx::GFXTextureFormat::R8G8B8A8_UNorm:      return RenderTextureColorFormat::RGBA8_UNorm;
        case gfx::GFXTextureFormat::R8G8B8A8_SRGB:       return RenderTextureColorFormat::RGBA8_SRGB;
        case gfx::GFXTextureFormat::B8G8R8A8_UNorm:      return RenderTextureColorFormat::BGRA8_UNorm;
        case gfx::GFXTextureFormat::R16G16B16A16_SFloat: return RenderTextureColorFormat::RGBA16_SFloat;
        case gfx::GFXTextureFormat::R32G32B32A32_SFloat: return RenderTextureColorFormat::RGBA32_SFloat;
        case gfx::GFXTextureFormat::B10G11R11_UFloat:    return RenderTextureColorFormat::R11G11B10_UFloat;
        default: return {};
        }
    }

    std::optional<RenderTextureDepthFormat> RenderTexture::FromGFXDepthFormat(gfx::GFXTextureFormat format)
    {
        switch (format)
        {
        case gfx::GFXTextureFormat::D32_SFloat:         return RenderTextureDepthFormat::D32_SFloat;
        case gfx::GFXTextureFormat::D32_SFloat_S8_UInt: return RenderTextureDepthFormat::D32_SFloat_S8_UInt;
        case gfx::GFXTextureFormat::D24_UNorm_S8_UInt:  return RenderTextureDepthFormat::D24_UNorm_S8_UInt;
        default: return {};
        }
    }

    std::shared_ptr<gfx::GFXTexture> RenderTexture::GetGFXTexture() const
    {
        if (m_renderTargets.empty())
            return nullptr;
        // Return first color attachment for preview/sampling
        return m_renderTargets[0];
    }

    void RenderTexture::Serialize(AssetSerializer* s)
    {
        base::Serialize(s);
        if (s->IsWrite)
        {
            s->Object->Add("Width", m_width);
            s->Object->Add("Height", m_height);
            s->Object->Add("SampleCount", (int)m_sampleCount);

            auto colorList = s->Object->New(ser::VarientType::Array);
            for (auto& fmt : *m_colorFormats)
            {
                colorList->Push(mkbox(fmt)->GetName());
            }
            s->Object->Add("ColorFormats", colorList);

            s->Object->Add("DepthFormat", mkbox(m_depthFormat)->GetName());
        }
        else
        {
            m_width = s->Object->At("Width")->AsInt();
            m_height = s->Object->At("Height")->AsInt();
            m_sampleCount = (uint32_t)s->Object->At("SampleCount")->AsInt();

            m_colorFormats->clear();
            auto colorList = s->Object->At("ColorFormats");
            for (int i = 0; i < colorList->GetCount(); ++i)
            {
                uint32_t value{};
                auto name = colorList->At(i)->AsString();
                if (Enum::StaticTryParse(cltypeof<BoxingRenderTextureColorFormat>(), name, &value))
                {
                    m_colorFormats->push_back((RenderTextureColorFormat)value);
                }
            }

            auto depthFormat = s->Object->At("DepthFormat")->AsString();
            uint32_t depthValue{};
            if (Enum::StaticTryParse(cltypeof<BoxingRenderTextureDepthFormat>(), depthFormat, &depthValue))
            {
                m_depthFormat = (RenderTextureDepthFormat)depthValue;
            }
        }
    }

    void RenderTexture::PostEditChange(FieldInfo* info)
    {
        base::PostEditChange(info);
        // Rebuild GPU resources when dimensions or attachments change
        if (IsCreatedGPUResource())
        {
            DestroyGPUResource();
            CreateGPUResource();
        }
    }

    bool RenderTexture::CreateGPUResource()
    {
        if (IsCreatedGPUResource())
            return true;

        auto gfx = Application::GetGfxApp();
        if (!gfx)
            return false;

        // Ensure at least one color format
        if (m_colorFormats->empty())
        {
            m_colorFormats->push_back(RenderTextureColorFormat::RGBA8_UNorm);
        }

        gfx::GFXSamplerConfig samplerCfg{};
        samplerCfg.Filter = gfx::GFXSamplerFilter::Linear;
        samplerCfg.AddressMode = gfx::GFXSamplerAddressMode::ClampToEdge;

        // Create color attachments (MRT)
        for (auto& fmt : *m_colorFormats)
        {
            auto rt = gfx->CreateRenderTarget(m_width, m_height, gfx::GFXTextureTargetType::ColorTarget, ToGFXFormat(fmt), samplerCfg, m_sampleCount, false);
            m_renderTargets.push_back(rt);
        }

        // Create depth attachment if specified
        if (m_depthFormat != RenderTextureDepthFormat::None)
        {
            auto depthFmt = ToGFXFormat(m_depthFormat);
            bool isDepthStencil = (m_depthFormat == RenderTextureDepthFormat::D32_SFloat_S8_UInt || m_depthFormat == RenderTextureDepthFormat::D24_UNorm_S8_UInt);
            auto targetType = isDepthStencil ? gfx::GFXTextureTargetType::DepthStencilTarget : gfx::GFXTextureTargetType::DepthTarget;
            auto rt = gfx->CreateRenderTarget(m_width, m_height, targetType, depthFmt, samplerCfg, m_sampleCount, false);
            m_renderTargets.push_back(rt);
        }

        // Build FBO
        std::vector<gfx::GFXTexture2DView_sp> views;
        for (auto& rt : m_renderTargets)
        {
            views.push_back(rt->Get2DView(0));
        }
        m_framebuffer = gfx->CreateFrameBufferObject(views);

        m_createdGPUResource = true;
        return true;
    }

    void RenderTexture::DestroyGPUResource()
    {
        if (!IsCreatedGPUResource())
            return;

        m_framebuffer.reset();
        m_renderTargets.clear();
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

    void RenderTexture::PostInitializeData(int32_t width, int32_t height)
    {
    }

    array_list<gfx::GFXTextureFormat> RenderTexture::GetSupportedDepthFormats()
    {
        auto gfx = Application::GetGfxApp();
        return gfx->GetSupportedDepthFormats();
    }

    bool RenderTexture::IsSupportedDepthFormat(gfx::GFXTextureFormat format)
    {
        for (auto element : GetSupportedDepthFormats())
        {
            if (element == format)
                return true;
        }
        return false;
    }

} // namespace pulsar
