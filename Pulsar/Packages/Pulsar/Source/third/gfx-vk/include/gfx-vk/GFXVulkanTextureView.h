#pragma once
#include "VulkanInclude.h"
#include <gfx/GFXTextureView.h>

namespace gfx
{
    class GFXVulkanTexture;
}
namespace gfx
{

    class GFXVulkanTexture2DView : public GFXTexture2DView
    {
    public:
        explicit GFXVulkanTexture2DView(GFXVulkanTexture* tex, uint32_t arrayIndex)
            : m_tex(tex), m_arrayIndex(arrayIndex)
        {}

        int32_t GetHeight() const override;
        int32_t GetWidth() const override;

        bool IsWritable() const override;
        bool IsReadable() const override;
        GFXTextureFormat GetFormat() const override;
        GFXTextureTargetType GetTargetType() const override;

        VkImage GetVkImage() const;

        GFXVulkanTexture* GetVkTexture() const { return m_tex; }
        GFXTexture* GetTexture() const override;

        VkImageView GetVkImageView() const;

        uint32_t GetBaseArrayIndex() const override { return m_arrayIndex; }

    protected:
        GFXVulkanTexture* m_tex;
        uint32_t m_arrayIndex;
    };
    GFX_DECL_SPTR(GFXVulkanTexture2DView);

}