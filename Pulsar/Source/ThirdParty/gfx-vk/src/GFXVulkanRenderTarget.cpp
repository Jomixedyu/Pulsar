#include "GFXVulkanRenderTarget.h"
#include "GFXVulkanApplication.h"
#include "BufferHelper.h"

namespace gfx
{
    GFXVulkanRenderTarget::GFXVulkanRenderTarget(
        GFXVulkanApplication* app, int32_t width, int32_t height,
        GFXRenderTargetType type, GFXTextureFormat format, const GFXSamplerConfig& samplerCfg)
        : m_app(app), m_type(type), m_mode(CreateMode_New),
        m_width(width), m_height(height)
    {
        m_imageFormat = BufferHelper::GetVkFormat(format);

        VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        VkImageAspectFlags aspect{};

        if (type == gfx::GFXRenderTargetType::Color)
        {
            m_imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
            aspect |= VK_IMAGE_ASPECT_COLOR_BIT;
        }
        else if (type == gfx::GFXRenderTargetType::DepthStencil)
        {
            m_imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
            aspect |= VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_DEPTH_BIT;
        }
        else if (type == gfx::GFXRenderTargetType::Depth)
        {
            m_imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
            usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
            aspect |= VK_IMAGE_ASPECT_DEPTH_BIT;
        }
        else if(type == gfx::GFXRenderTargetType::Stencil)
        {
            m_imageLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
            usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
            aspect |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
        else
        {
            assert(0);
        }

        BufferHelper::CreateImage(app, width, height,
            m_imageFormat,
            VK_IMAGE_TILING_OPTIMAL, usage,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_textureImage, m_textureImageMemory);
        
        BufferHelper::TransitionImageLayout(app, m_textureImage, m_imageFormat, 
            VK_IMAGE_LAYOUT_UNDEFINED, m_imageLayout);

        m_textureImageView = BufferHelper::CreateImageView(m_app, m_textureImage, m_imageFormat, aspect);
        m_textureSampler = BufferHelper::CreateTextureSampler(m_app);

    }
    GFXVulkanRenderTarget::~GFXVulkanRenderTarget()
    {
        if (m_mode & CreateMode_ShouldDeleted)
        {
            vkDestroyImage(m_app->GetVkDevice(), m_textureImage, nullptr);
            vkFreeMemory(m_app->GetVkDevice(), m_textureImageMemory, nullptr);
            vkDestroyImageView(m_app->GetVkDevice(), m_textureImageView, nullptr);
            
        }
        
    }
}