#include "GFXVulkanRenderTarget.h"
#include "GFXVulkanApplication.h"
#include "BufferHelper.h"

namespace gfx
{
    GFXVulkanRenderTarget::GFXVulkanRenderTarget(
        GFXVulkanApplication* app, int32_t width, int32_t height,
        GFXRenderTargetType type, GFXTextureFormat format, const GFXSamplerConfig& samplerCfg)
        : m_app(app), m_tex2d(nullptr), m_type(type)
    {

        VkImageLayout finalLayout;

        switch (type)
        {
        case gfx::GFXRenderTargetType::Color:
            finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            break;
        case gfx::GFXRenderTargetType::Depth:
            finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            break;
        default:
            assert(false);
            break;
        }

        m_tex2d = new GFXVulkanTexture2D(app, width, height, false, finalLayout, BufferHelper::GetVkFormat(format), samplerCfg);

    }
    GFXVulkanRenderTarget::~GFXVulkanRenderTarget()
    {
        //if (m_textureImage)
        //{
        //    vkDestroyImage(m_app->GetVkDevice(), m_textureImage, nullptr);
        //    vkFreeMemory(m_app->GetVkDevice(), m_textureMemory, nullptr);
        //}
    }
}