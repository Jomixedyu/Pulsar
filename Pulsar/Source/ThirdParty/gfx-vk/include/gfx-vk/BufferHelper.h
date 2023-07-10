#pragma once
#include "GFXVulkanApplication.h"

namespace gfx
{
    class BufferHelper
    {
    public:
        static uint32_t FindMemoryType(GFXVulkanApplication* app, uint32_t typeFilter, VkMemoryPropertyFlags properties);

        static void CreateBuffer(
            GFXVulkanApplication* app,
            VkDeviceSize size,
            VkBufferUsageFlags usage,
            VkMemoryPropertyFlags properties,
            VkBuffer& buffer,
            VkDeviceMemory& bufferMemory);

        static void TransferBuffer(GFXVulkanApplication* app, VkBuffer src, VkBuffer dest, VkDeviceSize size);

        static void DestroyBuffer(GFXVulkanApplication* app, VkBuffer buffer, VkDeviceMemory mem);

        static void CreateImage(
            GFXVulkanApplication* app,
            uint32_t width, uint32_t height,
            VkFormat format, VkImageTiling tiling,
            VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
            VkImage& image, VkDeviceMemory& imageMemory);

        static bool _HasStencilComponent(VkFormat format)
        {
            return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
        }

        static void TransitionImageLayout(
            GFXVulkanApplication* app,
            VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

        static void CopyBufferToImage(GFXVulkanApplication* app, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
        static VkImageView CreateImageView(GFXVulkanApplication* app, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
        static VkSampler CreateTextureSampler(
            GFXVulkanApplication* app, 
            VkFilter filter = VkFilter::VK_FILTER_LINEAR, 
            VkSamplerAddressMode addressMode = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);

        static VkFormat FindDepthFormat(GFXVulkanApplication* app);
    };
}