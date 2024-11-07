#include "ImageHelper.h"

#include "BufferHelper.h"

namespace gfx
{

    VkImageCreateInfo ImageHelper::ImageCreateInfo()
    {
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        return imageInfo;
    }

    VkImageCreateInfo ImageHelper::ImageCreateInfoTexture2D(
        int32_t width, int32_t height, VkFormat format, VkImageUsageFlags usage)
    {
        auto info = ImageCreateInfo();
        info.imageType = VK_IMAGE_TYPE_2D;
        info.extent.width = width;
        info.extent.height = height;
        info.format = format;
        info.usage = usage;
        info.arrayLayers = 1;
        return info;
    }

    VkImageCreateInfo ImageHelper::ImageCreateInfoCube(int32_t size, VkFormat format, VkImageUsageFlags usage)
    {
        auto info = ImageCreateInfo();
        info.imageType = VK_IMAGE_TYPE_2D;
        info.extent.width = size;
        info.extent.height = size;
        info.format = format;
        info.usage = usage;
        info.arrayLayers = 6;
        info.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
        return info;
    }

    void ImageHelper::CreateImage(GFXVulkanApplication* app,
        VkImageCreateInfo* info, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
    {
        if (vkCreateImage(app->GetVkDevice(), info, nullptr, &image) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create image!");
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(app->GetVkDevice(), image, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = gfx::BufferHelper::FindMemoryType(app, memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(app->GetVkDevice(), &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate image memory!");
        }

        vkBindImageMemory(app->GetVkDevice(), image, imageMemory, 0);
    }

    VkImageViewCreateInfo ImageHelper::ImageViewCreateInfo()
    {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        return viewInfo;
    }

    VkImageViewCreateInfo ImageHelper::ImageViewCreateInfoTexture2D(
        VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t index)
    {
        auto info = ImageViewCreateInfo();
        info.image = image;
        info.format = format;
        info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        info.subresourceRange.aspectMask = aspectFlags;
        info.subresourceRange.baseArrayLayer = index;
        info.subresourceRange.layerCount = 1;
        return info;
    }

    VkImageViewCreateInfo ImageHelper::ImageViewCreateInfoCube(VkImage image, VkFormat format)
    {
        auto info = ImageViewCreateInfo();
        info.image = image;
        info.format = format;
        info.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
        info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        info.subresourceRange.baseArrayLayer = 0;
        info.subresourceRange.layerCount = 6;
        return info;
    }

    VkImageView ImageHelper::CreateImageView(GFXVulkanApplication* app, VkImageViewCreateInfo* info)
    {
        VkImageView imageView;
        if (vkCreateImageView(app->GetVkDevice(), info, nullptr, &imageView) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create texture image view!");
        }
        return imageView;
    }
    VkImageAspectFlags ImageHelper::GetAspectFlags(GFXTextureTargetType type)
    {
        VkImageAspectFlags flags{};
        if (type == GFXTextureTargetType::ColorTarget)
        {
            flags = VK_IMAGE_ASPECT_COLOR_BIT;
        }
        else if (type == GFXTextureTargetType::DepthTarget)
        {
            flags = VK_IMAGE_ASPECT_DEPTH_BIT;
        }
        else if (type == GFXTextureTargetType::DepthStencilTarget)
        {
            flags = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        }
        else
        {
            flags = VK_IMAGE_ASPECT_COLOR_BIT;
        }
        return flags;
    }
    VkImageUsageFlags ImageHelper::GetImageUsageFlags(GFXTextureTargetType targetType)
    {
        VkImageUsageFlags flags = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

        if (targetType == GFXTextureTargetType::ColorTarget)
        {
            flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
            flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        }
        if (targetType == GFXTextureTargetType::DepthTarget)
        {
            flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
            flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        }
        if (targetType == GFXTextureTargetType::DepthStencilTarget)
        {
            flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
            flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        }
        return flags;
    }
} // namespace gfx