#include "GFXVulkanTexture.h"

#include "ImageHelper.h"

#include <cassert>
#include <gfx-vk/BufferHelper.h>
#include <gfx-vk/GFXVulkanCommandBuffer.h>
#include <gfx-vk/GFXVulkanTexture.h>
#include <gfx/GFXImage.h>
#include <stdexcept>

namespace gfx
{

    GFXVulkanTexture::~GFXVulkanTexture()
    {
        if (m_inited)
        {
            if (!m_isView)
            {
                vkDestroyImage(m_app->GetVkDevice(), m_textureImage, nullptr);
                vkFreeMemory(m_app->GetVkDevice(), m_textureImageMemory, nullptr);
                vkDestroyImageView(m_app->GetVkDevice(), m_textureImageView, nullptr);
            }
            vkDestroySampler(m_app->GetVkDevice(), m_textureSampler, nullptr);
        }
    }


    static VkImageLayout GetFinalImageLayout(GFXTextureTargetType type)
    {
        if (type == GFXTextureTargetType::ColorTarget)
        {
            return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        }
        if (type == GFXTextureTargetType::DepthStencilTarget)
        {
            return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        }
        if (type == GFXTextureTargetType::DepthTarget)
        {
            return VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
        }
        if (type == GFXTextureTargetType::None)
        {
            return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        }
        assert(false);
        return {};
    }

    GFXVulkanTexture::GFXVulkanTexture(GFXVulkanApplication* app, const GFXTextureCreateInfo& info)
        : base(info.width, info.height, info.depth, info.samplerCfg), m_isView(false), m_app(app)
    {
        m_dataType = info.dataType;
        m_imageFormat = BufferHelper::GetVkFormat(info.format);
        m_targetType = info.targetType;

        m_usageFlags = ImageHelper::GetImageUsageFlags(m_targetType);
        m_aspectFlags = ImageHelper::GetAspectFlags(m_targetType);

        auto createInfo = ImageHelper::ImageCreateInfoTexture2D(m_width, m_height, m_imageFormat, m_usageFlags);
        ImageHelper::CreateImage(app, &createInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_textureImage, m_textureImageMemory);

        auto currentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        if (info.dataLength > 0)
        {
            BufferHelper::TransitionImageLayout(app, m_textureImage, m_imageFormat, m_aspectFlags, currentLayout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
            currentLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

            VkBuffer stagingBuffer;
            VkDeviceMemory stagingBufferMemory;
            VkDeviceSize imageSize = info.dataLength;

            auto stagingProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

            gfx::BufferHelper::CreateBuffer(app, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                            stagingProperties, stagingBuffer, stagingBufferMemory);

            void* data;
            vkMapMemory(app->GetVkDevice(), stagingBufferMemory, 0, imageSize, 0, &data);
            memcpy(data, info.imageData, static_cast<size_t>(imageSize));
            vkUnmapMemory(app->GetVkDevice(), stagingBufferMemory);
            BufferHelper::CopyBufferToImage(app, stagingBuffer, m_textureImage, m_width, m_height);

            vkDestroyBuffer(app->GetVkDevice(), stagingBuffer, nullptr);
            vkFreeMemory(app->GetVkDevice(), stagingBufferMemory, nullptr);
        }

        m_targetFinalLayout = GetFinalImageLayout(m_targetType);

        BufferHelper::TransitionImageLayout(app, m_textureImage, m_imageFormat, m_aspectFlags, currentLayout, m_targetFinalLayout);
        m_imageLayout = m_targetFinalLayout;

        auto viewInfo = ImageHelper::ImageViewCreateInfoTexture2D(m_textureImage, m_imageFormat, m_aspectFlags);
        m_textureImageView = ImageHelper::CreateImageView(app, &viewInfo);

        auto filter = BufferHelper::GetVkFilter(m_samplerConfig.Filter);
        auto addressMode = BufferHelper::GetVkAddressMode(m_samplerConfig.AddressMode);

        m_textureSampler = BufferHelper::CreateTextureSampler(m_app, filter, addressMode);

        m_inited = true;
    }

    GFXVulkanTexture::GFXVulkanTexture(
        GFXVulkanApplication* app, const GFXVulkanTextureProxyCreateInfo& info)
        : base(info.width, info.height, 1, {}),
          m_app(app), m_textureImage(info.image), m_textureImageMemory(VK_NULL_HANDLE), m_imageFormat(info.format),
          m_imageLayout(info.layout), m_targetType(info.usage), m_isView(true), m_targetFinalLayout(info.finalTargetLayout),
        m_dataType(info.dataType)
    {
        m_usageFlags = ImageHelper::GetImageUsageFlags(m_targetType);
        m_aspectFlags = ImageHelper::GetAspectFlags(info.usage);

        m_textureImageView = info.view;
        m_textureSampler = BufferHelper::CreateTextureSampler(m_app, VkFilter::VK_FILTER_LINEAR, VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_REPEAT);
        m_inited = true;
    }

    GFXTexture2DView_sp GFXVulkanTexture::Get2DView(size_t index)
    {
        assert(m_dataType != GFXTextureDataType::None);
        if (m_2dviews.contains(index))
        {
            return m_2dviews[index];
        }

        if (m_dataType == GFXTextureDataType::Texture2D)
        {
            m_2dviews[index] = gfxmksptr(new GFXVulkanTexture2DView(this, 0));
        }
        else if (m_dataType == GFXTextureDataType::TextureCube)
        {
            assert(false);
        }
        else if (m_dataType == GFXTextureDataType::Texture2DArray)
        {
            assert(false);
        }

        return m_2dviews[index];
    }

    GFXTextureFormat GFXVulkanTexture::GetFormat() const
    {
        return BufferHelper::GetTextureFormat(m_imageFormat);
    }
    void GFXVulkanTexture::SetImageLayout(VkImageLayout layout)
    {
        m_imageLayout = layout;
    }
    void GFXVulkanTexture::TransitionLayout(VkCommandBuffer cmd, VkImageLayout layout)
    {
        if (m_imageLayout == layout)
        {
            return;
        }
        BufferHelper::TransitionImageLayout(cmd, this, layout);
        m_imageLayout = layout;
    }

} // namespace gfx