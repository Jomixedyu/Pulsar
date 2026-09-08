#include "Pulsar/Assets/RenderTextureCube.h"

#include "Application.h"
#include <Pulsar/Rendering/RenderThread.h>
#include <Pulsar/Rendering/TextureProxy.h>

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
        auto* renderThread = Application::GetRenderThread();

        gfx::GFXSamplerConfig sampler{};
        m_proxy = std::make_shared<rendering::TextureProxy>(m_width, sampler);

        auto proxy = m_proxy;
        renderThread->EnqueueUpdate_AnyThread(
            [proxy = std::move(proxy)](gfx::GFXResourceManager*) mutable
            {
                proxy->OnCreateResource();
            });

        m_isCreated = true;
        return true;
    }

    void RenderTextureCube::DestroyGPUResource()
    {
        if (!m_isCreated)
        {
            return;
        }
        m_isCreated = false;
        if (auto proxy = std::move(m_proxy))
        {
            Application::GetRenderThread()->EnqueueDestroy_AnyThread(
                [proxy = std::move(proxy)](gfx::GFXResourceManager*) mutable
                {
                    proxy->OnDestroyResource();
                });
        }
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

    gfx::TextureHandle RenderTextureCube::GetTextureHandle() const
    {
        return m_proxy ? m_proxy->GetTextureHandle() : gfx::TextureHandle{};
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
