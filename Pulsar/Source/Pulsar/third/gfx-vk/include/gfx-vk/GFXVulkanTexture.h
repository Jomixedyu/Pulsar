#pragma once
#include "VulkanInclude.h"

#include <gfx/GFXTexture.h>
#include <map>

namespace gfx
{
    class GFXVulkanApplication;

    struct GFXVulkanTextureProxyCreateInfo
    {
        int width;
        int height;
        VkFormat format;
        VkImage image;
        GFXTextureTargetType usage;
        VkImageLayout layout;
        VkImageView view;
        VkImageLayout finalTargetLayout;
        GFXTextureDataType dataType;
    };

    class GFXVulkanTexture : public GFXTexture
    {
        using base = GFXTexture;
    public:
        using DataDeleter = void(*)(uint8_t*);
        virtual const type_info& GetClassId() const override { return typeid(GFXVulkanTexture); }

    public:
        virtual ~GFXVulkanTexture() override;

        //create by pic data
        GFXVulkanTexture(GFXVulkanApplication* app, const GFXTextureCreateInfo& info);

        //view
        GFXVulkanTexture(GFXVulkanApplication* app, const GFXVulkanTextureProxyCreateInfo& info);

        GFXVulkanTexture(const GFXVulkanTexture&) = delete;
    public:

        GFXTexture2DView_sp Get2DView(size_t index) override;

        VkImage GetVkImage() const { return m_textureImage; }
        VkImageView GetVkImageView() const { return m_textureImageView; }
        VkSampler GetVkSampler() const { return m_textureSampler; }
        VkFormat GetVkImageFormat() const { return m_imageFormat; }
        VkImageLayout GetVkImageLayout() const { return m_imageLayout; }
        VkImageLayout GetVkTargetFinalLayout() const { return m_targetFinalLayout; }
        GFXTextureFormat GetFormat() const override;
        VkImageAspectFlags GetVkAspectFlags() const { return m_aspectFlags; }
        void SetImageLayout(VkImageLayout layout);

        bool IsTarget() const { return m_targetType != GFXTextureTargetType::None; }
        GFXTextureTargetType GetTargetType() const override { return m_targetType; }
        void TransitionLayout(VkCommandBuffer cmd, VkImageLayout layout);

    protected:
        GFXVulkanApplication* m_app;

        VkImage m_textureImage{};
        VkImageLayout m_imageLayout;
        VkImageLayout m_targetFinalLayout;
        VkDeviceMemory m_textureImageMemory{};
        VkImageView m_textureImageView{};
        VkSampler m_textureSampler{};
        VkFormat m_imageFormat;
        VkImageAspectFlags m_aspectFlags{};
        VkImageUsageFlags m_usageFlags{};
        GFXTextureDataType m_dataType;
        GFXTextureTargetType m_targetType;

        std::map<size_t, GFXTexture2DView_sp> m_2dviews;

        bool m_isView = false;
        bool m_isManaged = false;
        bool m_inited = false;
    };


}