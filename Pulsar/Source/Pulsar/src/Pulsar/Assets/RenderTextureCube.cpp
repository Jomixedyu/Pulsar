#include "Pulsar/Assets/RenderTextureCube.h"

#include "Application.h"

namespace pulsar
{

    RenderTextureCube::RenderTextureCube()
        : m_width(1024)
    {
    }

    bool RenderTextureCube::CreateGPUResource()
    {
        if (m_isCreated)
        {
            return true;
        }
        m_isCreated = true;
        if (!m_proxy)
            m_proxy = mksptr(new RenderProxyRenderTextureCube(this));
        m_proxy->InitRHI();
        return true;
    }

    void RenderTextureCube::DestroyGPUResource()
    {
        if (!m_isCreated)
        {
            return;
        }
        m_isCreated = false;
        if (m_proxy)
            m_proxy->ReleaseRHI();
        m_proxy.reset();
    }

    bool RenderTextureCube::IsCreatedGPUResource() const
    {
        return m_isCreated;
    }
    int32_t RenderTextureCube::GetWidth() const
    {
        return m_width;
    }
    int32_t RenderTextureCube::GetHeight() const
    {
        return m_width;
    }

    std::shared_ptr<gfx::GFXTexture> RenderTextureCube::GetGFXTexture() const
    {
        if (m_proxy)
            return m_proxy->GetGFXTexture();
        return nullptr;
    }

    void RenderTextureCube::PostEditChange(FieldInfo* info)
    {
        base::PostEditChange(info);
        if (info->GetName() == NAMEOF(m_width))
        {
            SetWidth(m_width);
        }
    }
    void RenderTextureCube::SetWidth(int32_t width)
    {
        m_width = width;
        DestroyGPUResource();
        CreateGPUResource();
    }
} // namespace pulsar
