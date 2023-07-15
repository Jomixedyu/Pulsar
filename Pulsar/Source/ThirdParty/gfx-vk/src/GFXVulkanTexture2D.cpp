#include "GFXVulkanTexture2D.h"
#include "GFXVulkanTexture2D.h"
#include <gfx-vk/BufferHelper.h>
#include <gfx-vk/GFXVulkanTexture2D.h>
#include <gfx-vk/GFXVulkanCommandBuffer.h>
#include <stdexcept>
#include <stb_image.h>
#include <cassert>

namespace gfx
{

    GFXVulkanTexture2D::~GFXVulkanTexture2D()
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

    GFXVulkanTexture2D::GFXVulkanTexture2D(
        GFXVulkanApplication* app,
        const uint8_t* imageData,
        int32_t width, int32_t height, int32_t channel,
        VkFormat format,
        bool enableReadWrite, const GFXSamplerConfig& samplerCfg)
        :
        base(width, height, channel, samplerCfg, enableReadWrite),
        m_app(app), m_imageFormat(format)
    {
        //读取图片至暂存buffer
        //创建图片资源
        //变换到最佳布局
        //暂存buffer拷贝到图像资源

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        VkDeviceSize imageSize = width * height * channel;

        gfx::BufferHelper::CreateBuffer(app, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void* data;
        vkMapMemory(app->GetVkDevice(), stagingBufferMemory, 0, imageSize, 0, &data);
        memcpy(data, imageData, static_cast<size_t>(imageSize));
        vkUnmapMemory(app->GetVkDevice(), stagingBufferMemory);


        BufferHelper::CreateImage(app, width, height,
            m_imageFormat,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_textureImage, m_textureImageMemory);

        BufferHelper::TransitionImageLayout(app, m_textureImage, m_imageFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        BufferHelper::CopyBufferToImage(app, stagingBuffer, m_textureImage, static_cast<uint32_t>(width), static_cast<uint32_t>(height));
        //变换为采样最佳
        BufferHelper::TransitionImageLayout(app, m_textureImage, m_imageFormat, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        m_imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        vkDestroyBuffer(app->GetVkDevice(), stagingBuffer, nullptr);
        vkFreeMemory(app->GetVkDevice(), stagingBufferMemory, nullptr);

        m_textureImageView = BufferHelper::CreateImageView(m_app, m_textureImage, m_imageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
        m_textureSampler = BufferHelper::CreateTextureSampler(m_app);

        m_inited = true;
    }

    GFXVulkanTexture2D::GFXVulkanTexture2D(
        GFXVulkanApplication* app, int32_t width, int32_t height, 
        bool enableReadWrite, VkImageLayout layout, VkFormat format, const GFXSamplerConfig& samplerCfg)
        : base(width, height, 0, samplerCfg, enableReadWrite),
        m_app(app), m_imageLayout(layout), m_imageFormat(format)
    {

        BufferHelper::CreateImage(app, width, height,
            m_imageFormat,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_textureImage, m_textureImageMemory);

        BufferHelper::TransitionImageLayout(app, m_textureImage, m_imageFormat, VK_IMAGE_LAYOUT_UNDEFINED, layout);

        m_textureImageView = BufferHelper::CreateImageView(m_app, m_textureImage, m_imageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
        m_textureSampler = BufferHelper::CreateTextureSampler(m_app);

        m_inited = true;
    }

    GFXVulkanTexture2D::GFXVulkanTexture2D(
        GFXVulkanApplication* app,
        int32_t width, int32_t height, int32_t channel,
        VkFormat format, VkImage image, VkDeviceMemory memory, VkImageView imageView,
        bool enableReadWrite, VkImageLayout layout, const GFXSamplerConfig& samplerCfg)
        : 
        base(width, height, channel, samplerCfg, enableReadWrite),
        m_app(app), m_textureImage(image), m_textureImageMemory(memory), m_textureImageView(imageView),
        m_imageLayout(layout), m_imageFormat(format), m_isManaged(true)
    {
        auto filter = BufferHelper::GetVkFilter(samplerCfg.Filter);
        auto addressMode = BufferHelper::GetVkAddressMode(samplerCfg.AddressMode);

        m_textureSampler = BufferHelper::CreateTextureSampler(m_app, filter, addressMode);
        m_inited = true;
    }

    GFXVulkanTexture2D::GFXVulkanTexture2D(
        GFXVulkanApplication* app, int32_t width, int32_t height, int32_t channel, 
        VkFormat format, VkImage image, VkImageView imageView, VkImageLayout layout)
        :
        base(width, height, channel, {}, false),
        m_app(app), m_textureImage(image), m_textureImageMemory(VK_NULL_HANDLE), m_imageFormat(format),
        m_textureImageView(imageView), m_imageLayout(layout), m_isView(true)
    {
        m_textureSampler = BufferHelper::CreateTextureSampler(m_app, VkFilter::VK_FILTER_LINEAR, VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_REPEAT);
        m_inited = true;
    }

    const uint8_t* GFXVulkanTexture2D::GetData() const
    {
        return nullptr;
    }



    static std::vector<uint8_t> _FloatToByte(float* data, size_t len)
    {
        std::vector<uint8_t> newData(len);

        for (size_t i = 0; i < len; i++)
        {
            newData[i] = (uint8_t)(data[i] * 255);
        }
        return newData;
    }
    std::shared_ptr<GFXVulkanTexture2D> GFXVulkanTexture2D::CreateFromMemory(
        GFXVulkanApplication* app, const uint8_t* fileData, int32_t length, bool enableReadWrite, 
        GFXTextureFormat format, const GFXSamplerConfig& samplerCfg)
    {
        int x, y, channel;
        std::vector<uint8_t> buffer;

        switch (format)
        {
        case gfx::GFXTextureFormat::R8G8B8A8:
        {
            auto loaded = stbi_loadf_from_memory(fileData, length, &x, &y, &channel, 4);
            channel = 4;
            buffer = _FloatToByte(loaded, x * y * 4);
            stbi_image_free(loaded);
        }
        break;
        case gfx::GFXTextureFormat::R8G8B8A8_SRGB:
        {
            auto loaded = stbi_load_from_memory(fileData, length, &x, &y, &channel, 4);
            channel = 4;
            auto len = x * y * 4;
            buffer.resize(len);
            memcpy(buffer.data(), loaded, len);
            stbi_image_free(loaded);
        }
        break;
        default:
            assert(false);
            break;
        }

        auto tex = new GFXVulkanTexture2D(app, buffer.data(), x, y, channel, BufferHelper::GetVkFormat(format), enableReadWrite, samplerCfg);

        return std::shared_ptr<GFXVulkanTexture2D>(tex);
    }

}