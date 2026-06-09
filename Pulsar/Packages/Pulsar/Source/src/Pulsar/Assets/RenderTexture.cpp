#include "Assets/RenderTexture.h"
#include <Pulsar/Application.h>
#include <gfx/GFXResourceManager.h>
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

    gfx::TextureHandle RenderTexture::GetTextureHandle() const
    {
        if (m_renderTargetHandles.empty())
            return {};
        // Return first color attachment for preview/sampling
        return m_renderTargetHandles[0];
    }

    std::shared_ptr<gfx::GFXTexture2DView> RenderTexture::GetGfxRenderTarget0() const
    {
        if (m_renderTargetHandles.empty())
            return nullptr;
        auto* resMgr = Application::GetGfxApp()->GetResourceManager();
        auto tex = resMgr->GetTextureShared(m_renderTargetHandles[0]);
        return tex ? tex->Get2DView(0) : nullptr;
    }

    std::shared_ptr<gfx::GFXFrameBufferObject> RenderTexture::GetGfxFrameBufferObject() const
    {
        auto* resMgr = Application::GetGfxApp()->GetResourceManager();
        return resMgr->GetFrameBufferObjectShared(m_framebufferHandle);
    }

    array_list<gfx::GFXTexture_sp> RenderTexture::GetRenderTargets() const
    {
        auto* resMgr = Application::GetGfxApp()->GetResourceManager();
        array_list<gfx::GFXTexture_sp> result;
        result.reserve(m_renderTargetHandles.size());
        for (auto& h : m_renderTargetHandles)
        {
            result.push_back(resMgr->GetTextureShared(h));
        }
        return result;
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

        auto* resMgr = gfx->GetResourceManager();

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
            gfx::GFXTextureCreateDesc desc{};
            desc.Width = m_width;
            desc.Height = m_height;
            desc.TargetType = gfx::GFXTextureTargetType::ColorTarget;
            desc.Format = ToGFXFormat(fmt);
            desc.SamplerCfg = samplerCfg;
            desc.SampleCount = m_sampleCount;
            desc.IsTransientAttachment = false;

            auto h = resMgr->AllocHandle<gfx::TextureHandle>();
            resMgr->CreateRenderTarget(h, desc);
            m_renderTargetHandles.push_back(h);
        }

        // Create depth attachment if specified
        if (m_depthFormat != RenderTextureDepthFormat::None)
        {
            auto depthFmt = ToGFXFormat(m_depthFormat);
            bool isDepthStencil = (m_depthFormat == RenderTextureDepthFormat::D32_SFloat_S8_UInt || m_depthFormat == RenderTextureDepthFormat::D24_UNorm_S8_UInt);
            auto targetType = isDepthStencil ? gfx::GFXTextureTargetType::DepthStencilTarget : gfx::GFXTextureTargetType::DepthTarget;

            gfx::GFXTextureCreateDesc desc{};
            desc.Width = m_width;
            desc.Height = m_height;
            desc.TargetType = targetType;
            desc.Format = depthFmt;
            desc.SamplerCfg = samplerCfg;
            desc.SampleCount = m_sampleCount;
            desc.IsTransientAttachment = false;

            auto h = resMgr->AllocHandle<gfx::TextureHandle>();
            resMgr->CreateRenderTarget(h, desc);
            m_renderTargetHandles.push_back(h);
        }

        // Build FBO
        std::vector<gfx::GFXTexture2DView_sp> views;
        for (auto& h : m_renderTargetHandles)
        {
            auto tex = resMgr->GetTextureShared(h);
            views.push_back(tex ? tex->Get2DView(0) : nullptr);
        }
        m_framebufferHandle = resMgr->AllocHandle<gfx::FrameBufferObjectHandle>();
        resMgr->CreateFrameBufferObject(m_framebufferHandle, views);

        m_createdGPUResource = true;
        return true;
    }

    void RenderTexture::DestroyGPUResource()
    {
        if (!IsCreatedGPUResource())
            return;

        auto* resMgr = Application::GetGfxApp()->GetResourceManager();
        if (m_framebufferHandle.IsValid())
        {
            resMgr->Destroy(m_framebufferHandle);
            m_framebufferHandle = {};
        }
        for (auto& h : m_renderTargetHandles)
        {
            resMgr->Destroy(h);
        }
        m_renderTargetHandles.clear();
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
