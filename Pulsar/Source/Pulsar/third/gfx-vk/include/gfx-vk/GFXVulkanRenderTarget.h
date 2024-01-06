#pragma once
#include <gfx/GFXRenderTarget.h>
#include "VulkanInclude.h"
#include "GFXVulkanTexture2D.h"

namespace gfx
{
    class GFXVulkanApplication;

    class GFXVulkanRenderTarget : public GFXRenderTarget
    {
        enum CreateMode
        {
            CreateMode_View = 0,
            CreateMode_New = 1,
            CreateMode_Managed = 1 << 1,
            CreateMode_ShouldDeleted = CreateMode_New | CreateMode_Managed
        };
    public:
        static inline size_t StaticTexutreType() { return 2; }
        virtual size_t GetTextureType() const override { return StaticTexutreType(); }
    public:
        // create view
        GFXVulkanRenderTarget(
            int32_t width, int32_t height,
            VkImage image, VkImageView imageView, VkFormat format, VkImageLayout layout, GFXRenderTargetType type);

        // create new
        GFXVulkanRenderTarget(
            GFXVulkanApplication* app, int32_t width, int32_t height, 
            GFXRenderTargetType type, GFXTextureFormat format, const GFXSamplerConfig& samplerCfg);

        virtual ~GFXVulkanRenderTarget() override;

        GFXVulkanRenderTarget(const GFXVulkanRenderTarget&) = delete;
        GFXVulkanRenderTarget(GFXVulkanRenderTarget&&) = delete;

    public:
        virtual GFXRenderTargetType GetRenderTargetType() const override { return m_type; }
        VkImageLayout GetVkImageLayout() const { return m_imageLayout; }
        VkImageLayout GetVkImageFinalLayout() const { return m_imageFinalLayout; }
        VkImageView GetVkImageView() const { return m_textureImageView; }
        VkImage GetVkImage() const { return m_textureImage; }
        VkSampler GetVkSampler() const { return m_textureSampler; }
        VkFormat GetVkFormat() const { return m_imageFormat; }
        VkImageAspectFlags GetAspectFlags() const { return m_aspectFlags; }
        virtual int32_t GetWidth() const override { return m_width; }
        virtual int32_t GetHeight() const override { return m_height; }

        void CmdLayoutTransition(VkCommandBuffer cmd, VkImageLayout newLayout);
    protected:
        
        GFXRenderTargetType m_type;

    private:
        CreateMode m_mode;

        int32_t m_width;
        int32_t m_height;

        GFXVulkanApplication* m_app;
        VkImage m_textureImage;

        VkDeviceMemory m_textureImageMemory;
        VkImageView m_textureImageView;
        VkSampler m_textureSampler{};
        VkFormat m_imageFormat;
        VkImageAspectFlags m_aspectFlags;
    public:
        VkImageLayout m_imageLayout{};
        VkImageLayout m_imageFinalLayout{};
    };
}