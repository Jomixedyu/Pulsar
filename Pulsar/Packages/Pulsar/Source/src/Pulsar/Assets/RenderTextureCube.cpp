#include "Pulsar/Assets/RenderTextureCube.h"

#include "Application.h"
#include <gfx/GFXResourceManager.h>

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
        auto* resMgr = Application::GetGfxApp()->GetResourceManager();
        m_textureHandle = resMgr->AllocHandle<gfx::TextureHandle>();
        resMgr->CreateTextureCube(m_textureHandle, m_width);

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
        if (m_textureHandle.IsValid())
        {
            auto* resMgr = Application::GetGfxApp()->GetResourceManager();
            resMgr->Destroy(m_textureHandle);
            m_textureHandle = {};
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