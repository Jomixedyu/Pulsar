#pragma once
#include "GFXVulkanApplication.h"
#include <vector>

namespace gfx
{
    class ImageHelper final
    {
    public:
        static VkImageCreateInfo ImageCreateInfo();

        static VkImageCreateInfo ImageCreateInfoTexture2D(
            int32_t width, int32_t height, VkFormat format, VkImageUsageFlags usage);

        static VkImageCreateInfo ImageCreateInfoCube(
            int32_t size, VkFormat format, VkImageUsageFlags usage);

        static void CreateImage(GFXVulkanApplication* app,
            VkImageCreateInfo* info, VkMemoryPropertyFlags properties,
            VkImage& image, VkDeviceMemory& imageMemory);


        static VkImageViewCreateInfo ImageViewCreateInfo();

        static VkImageViewCreateInfo ImageViewCreateInfoTexture2D(
            VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t index = 0);

        static VkImageViewCreateInfo ImageViewCreateInfoCube(
            VkImage image, VkFormat format);

        static VkImageView CreateImageView(GFXVulkanApplication* app, VkImageViewCreateInfo* info);

        static VkImageAspectFlags GetAspectFlags(GFXTextureTargetType type);

        static VkImageUsageFlags GetImageUsageFlags(GFXTextureTargetType targetType);
    };
}