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
        m_texture = Application::GetGfxApp()->CreateTextureCube(m_width);

        m_isCreated = true;
    }

    void RenderTextureCube::DestroyGPUResource()
    {
        if (!m_isCreated)
        {
            return;
        }
        m_isCreated = false;
        m_texture.reset();
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