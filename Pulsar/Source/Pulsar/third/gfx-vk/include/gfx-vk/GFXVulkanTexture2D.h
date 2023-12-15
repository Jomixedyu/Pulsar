#pragma once
#include "VulkanInclude.h"
#include <gfx/GFXTexture2D.h>

namespace gfx
{
    class GFXVulkanApplication;

    class GFXVulkanTexture2D : public GFXTexture2D
    {
        using base = GFXTexture2D;
    public:
        using DataDeleter = void(*)(uint8_t*);
        static inline size_t StaticTexutreType() { return 1; }
        virtual size_t GetTextureType() const override { return StaticTexutreType(); }
    public:
        static std::shared_ptr<GFXVulkanTexture2D> CreateFromImageData(
            GFXVulkanApplication* app, const uint8_t* imageData, int32_t width, int32_t height, int32_t channel,
            bool enableReadWrite, GFXTextureFormat format, const GFXSamplerConfig& samplerCfg);

        static std::shared_ptr<GFXVulkanTexture2D> CreateFromMemory(
            GFXVulkanApplication* app, const uint8_t* fileData, size_t length, bool enableReadWrite,
            GFXTextureFormat format, const GFXSamplerConfig& samplerCfg);

    public:
        virtual ~GFXVulkanTexture2D() override;

        //create by pic data
        GFXVulkanTexture2D(
            GFXVulkanApplication* app,
            const uint8_t* imageData,
            int32_t width, int32_t height, int32_t channel,
            VkFormat format,
            bool enableReadWrite,
            const GFXSamplerConfig& samplerCfg);

        //create empty
        GFXVulkanTexture2D(
            GFXVulkanApplication* app, int32_t width, int32_t height, 
            bool enableReadWrite, VkImageLayout layout, VkFormat format, const GFXSamplerConfig& samplerCfg);

        //managed
        GFXVulkanTexture2D(GFXVulkanApplication* app, int32_t width, int32_t height, int32_t channel,
            VkFormat format, VkImage image, VkDeviceMemory memory, VkImageView imageView, bool enableReadWrite, VkImageLayout layout,
            const GFXSamplerConfig& samplerCfg);

        //view
        GFXVulkanTexture2D(GFXVulkanApplication* app, int32_t width, int32_t height, int32_t channel,
            VkFormat format, VkImage image, VkImageView imageView, VkImageLayout layout);

        GFXVulkanTexture2D(const GFXVulkanTexture2D&) = delete;
    public:
        virtual const uint8_t* GetData() const override;
    public:

        VkImage GetVkImage() const { return m_textureImage; }
        VkImageView GetVkImageView() const { return m_textureImageView; }
        VkSampler GetVkSampler() const { return m_textureSampler; }
        VkFormat GetVkImageFormat() const { return m_imageFormat; }
        VkImageLayout GetVkImageLayout() const { return m_imageLayout; }
    protected:
        GFXVulkanApplication* m_app;

        VkImage m_textureImage;
        VkImageLayout m_imageLayout;
        VkDeviceMemory m_textureImageMemory;
        VkImageView m_textureImageView;
        VkSampler m_textureSampler;
        VkFormat m_imageFormat;

        bool m_isView = false;
        bool m_isManaged = false;
        bool m_inited = false;
    };
}