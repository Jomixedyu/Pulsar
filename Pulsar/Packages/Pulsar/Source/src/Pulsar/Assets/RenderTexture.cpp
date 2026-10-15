#include "Assets/RenderTexture.h"
#include <Pulsar/Application.h>
#include <Pulsar/Rendering/RenderThread.h>
#include <Pulsar/Rendering/RenderTextureProxy.h>
#include <optional>

namespace pulsar
{
    RenderTexture::RenderTexture()
    {
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

    gfx::GFXTexturePtr RenderTexture::GetGfxTexture() const
    {
        return m_proxy ? m_proxy->GetColorTexture() : nullptr;
    }

    std::shared_ptr<gfx::GFXTexture2DView> RenderTexture::GetGfxColorTextureView() const
    {
        return m_proxy ? m_proxy->GetColorTextureView() : nullptr;
    }

    std::shared_ptr<gfx::GFXFrameBufferObject> RenderTexture::GetGfxFrameBufferObject() const
    {
        return m_proxy ? m_proxy->GetFrameBufferObject() : nullptr;
    }

    array_list<gfx::GFXTexturePtr> RenderTexture::GetFramebufferAttachments() const
    {
        return m_proxy ? m_proxy->GetFramebufferAttachments() : array_list<gfx::GFXTexturePtr>{};
    }

    void RenderTexture::Serialize(AssetSerializer* s)
    {
        base::Serialize(s);
        if (s->IsWrite)
        {
            s->Object->Add("Width", m_width);
            s->Object->Add("Height", m_height);
            s->Object->Add("SampleCount", (int)m_sampleCount);

            s->Object->Add("ColorFormat", mkbox(m_colorFormat)->GetName());

            s->Object->Add("DepthFormat", mkbox(m_depthFormat)->GetName());
        }
        else
        {
            m_width = s->Object->At("Width")->AsInt();
            m_height = s->Object->At("Height")->AsInt();
            m_sampleCount = (uint32_t)s->Object->At("SampleCount")->AsInt();

            if (auto colorFormat = s->Object->At("ColorFormat"))
            {
                uint32_t value{};
                if (Enum::StaticTryParse(cltypeof<BoxingRenderTextureColorFormat>(), colorFormat->AsString(), &value))
                {
                    m_colorFormat = (RenderTextureColorFormat)value;
                }
            }
            else if (auto colorList = s->Object->At("ColorFormats"))
            {
                if (colorList->GetCount() > 0)
                {
                    uint32_t value{};
                    if (Enum::StaticTryParse(cltypeof<BoxingRenderTextureColorFormat>(), colorList->At(0)->AsString(), &value))
                        m_colorFormat = (RenderTextureColorFormat)value;
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
        // Dimensions / formats / samples changed: rebuild on EnsureGPUResource.
        m_gpuResourceDirty = true;
        EnsureGPUResource();
    }

    bool RenderTexture::EnsureGPUResource()
    {
        if (m_createdGPUResource && !m_gpuResourceDirty)
            return true;
        m_gpuResourceDirty = false;

        auto gfx = Application::GetGfxApp();
        if (!gfx)
            return false;

        // Init is re-init safe: a rebuild reuses the same proxy object, so
        // render-side holders see the new targets automatically. No wait-idle:
        // the targets become available at the next frame-head queue drain.
        if (!m_proxy)
            m_proxy = std::make_shared<rendering::RenderTextureProxy>();

        auto proxy = m_proxy;
        const int32_t w = m_width, h = m_height;
        const uint32_t samples = m_sampleCount;
        const auto colorFormat = m_colorFormat;
        const auto depthFormat = m_depthFormat;
        Application::GetRenderThread()->EnqueueUpdate_AnyThread(
            [proxy = std::move(proxy), w, h, samples, colorFormat, depthFormat](gfx::GFXResourceRegistry*) mutable
            {
                proxy->Init(w, h, samples, colorFormat, depthFormat);
            });

        m_createdGPUResource = true;
        return true;
    }

    void RenderTexture::DestroyGPUResource()
    {
        if (!IsCreatedGPUResource())
            return;

        auto proxy = std::move(m_proxy);
        if (!proxy)
            return;

        Application::GetRenderThread()->EnqueueDestroy_AnyThread(
            [proxy = std::move(proxy)](gfx::GFXResourceRegistry*)
            {
                proxy->Release();
            });
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
