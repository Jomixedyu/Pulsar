#include "GFXVulkanTextureView.h"
#include "GFXVulkanTexture.h"
#include "BufferHelper.h"

namespace gfx
{

    int32_t GFXVulkanTexture2DView::GetHeight() const
    {
        return m_tex->GetHeight();
    }
    int32_t GFXVulkanTexture2DView::GetWidth() const
    {
        return m_tex->GetWidth();
    }
    bool GFXVulkanTexture2DView::IsWritable() const
    {
        return m_tex->IsTarget();
    }
    bool GFXVulkanTexture2DView::IsReadable() const
    {
        return false;
    }
    GFXTextureFormat GFXVulkanTexture2DView::GetFormat() const
    {
        return m_tex->GetFormat();
    }
    GFXTextureTargetType GFXVulkanTexture2DView::GetTargetType() const
    {
        return m_tex->GetTargetType();
    }
    VkImage GFXVulkanTexture2DView::GetVkImage() const
    {
        return m_tex->GetVkImage();
    }
    GFXTexture* GFXVulkanTexture2DView::GetTexture() const
    {
        return m_tex;
    }
    VkImageView GFXVulkanTexture2DView::GetVkImageView() const
    {
        return m_tex->GetVkImageView();
    }
} // namespace gfx