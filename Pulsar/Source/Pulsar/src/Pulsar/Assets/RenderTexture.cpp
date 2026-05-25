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
        if (!m_proxy)
            return nullptr;
        auto rt0 = m_proxy->GetRenderTarget0();
        return rt0;
    }

    std::shared_ptr<gfx::GFXTexture2DView> RenderTexture::GetGfxRenderTarget0() const
    {
        if (!m_proxy)
            return nullptr;
        auto rt0 = m_proxy->GetRenderTarget0();
        return rt0 ? rt0->Get2DView(0) : nullptr;
    }

    std::shared_ptr<gfx::GFXFrameBufferObject> RenderTexture::GetGfxFrameBufferObject() const
    {
        return m_proxy ? m_proxy->GetFrameBufferObject() : nullptr;
    }

    const array_list<gfx::GFXTexture_sp>& RenderTexture::GetRenderTargets() const
    {
        static array_list<gfx::GFXTexture_sp> empty;
        return m_proxy ? m_proxy->GetRenderTargets() : empty;
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

        m_createdGPUResource = true;
        if (!m_proxy)
            m_proxy = mksptr(new RenderProxyRenderTexture(this));
        m_proxy->InitRHI();
        return true;
    }

    void RenderTexture::DestroyGPUResource()
    {
        if (!IsCreatedGPUResource())
            return;

        m_createdGPUResource = false;
        if (m_proxy)
            m_proxy->ReleaseRHI();
        m_proxy.reset();
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
