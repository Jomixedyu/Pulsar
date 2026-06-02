#pragma once
#include "GFXVulkanApplication.h"
#include <vector>

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

        static void TransitionImageLayout(
            GFXVulkanApplication* app,
            VkImage image, VkFormat format, VkImageAspectFlags aspect, VkImageLayout oldLayout, VkImageLayout newLayout);

        static void TransitionImageLayout(
            VkCommandBuffer cmd, GFXVulkanTexture* tex, VkImageLayout newLayout);

        static VkPipelineStageFlags GetStageFlagsForLayout(VkImageLayout layout);

        static VkAccessFlags GetAccessMaskForLayout(VkImageLayout layout);

        static void CopyBufferToImage(GFXVulkanApplication* app, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);


        static VkSampler CreateTextureSampler(
            GFXVulkanApplication* app, 
            VkFilter filter = VkFilter::VK_FILTER_LINEAR, 
            VkSamplerAddressMode addressMode = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);

        static std::vector<VkFormat> FindDepthFormats(GFXVulkanApplication* app, bool assertEmpty = false);


        static VkFilter GetVkFilter(GFXSamplerFilter filter);
        static VkSamplerAddressMode GetVkAddressMode(GFXSamplerAddressMode mode);

        static VkFormat GetVkFormat(GFXTextureFormat format);
        static GFXTextureFormat GetTextureFormat(VkFormat format);
    };
}