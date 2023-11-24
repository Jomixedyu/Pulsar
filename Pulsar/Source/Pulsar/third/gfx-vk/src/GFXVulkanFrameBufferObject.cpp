#include "GFXVulkanFrameBufferObject.h"
#include "GFXVulkanApplication.h"
#include "PhysicalDeviceHelper.h"
#include "BufferHelper.h"
#include <array>
#include <cassert>

namespace gfx
{
    static int32_t idNext = 0;

    GFXVulkanFrameBufferObject::GFXVulkanFrameBufferObject(
        GFXVulkanApplication* app,
        const std::vector<GFXRenderTarget*>& renderTargets,
        const std::shared_ptr<GFXVulkanRenderPass>& renderPass)
        : m_app(app), m_renderTargets(renderTargets), m_renderPass(renderPass)
    {
        assert(renderTargets.size() != 0);
        auto& first = renderTargets[0];
        m_width = first->GetWidth();
        m_height = first->GetHeight();

        m_id = ++idNext;

        this->InitRenderPass();
    }


    void GFXVulkanFrameBufferObject::InitRenderPass()
    {
        //create framebuffer
        std::vector<VkImageView> fbImageViews;

        for (auto& renderTarget : m_renderTargets)
        {
            fbImageViews.push_back(static_cast<GFXVulkanRenderTarget*>(renderTarget)->GetVkImageView());
        }

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_renderPass->GetVkRenderPass();
        framebufferInfo.attachmentCount = static_cast<uint32_t>(fbImageViews.size());
        framebufferInfo.pAttachments = fbImageViews.data();
        framebufferInfo.width = m_width;
        framebufferInfo.height = m_height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(m_app->GetVkDevice(), &framebufferInfo, nullptr, &m_frameBuffer) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create framebuffer!");
        }

    }

    void GFXVulkanFrameBufferObject::TermRenderPass()
    {
        vkDestroyFramebuffer(m_app->GetVkDevice(), m_frameBuffer, nullptr);
        m_frameBuffer = VK_NULL_HANDLE;
        m_renderPass.reset();
    }

    GFXVulkanFrameBufferObject::~GFXVulkanFrameBufferObject()
    {
        TermRenderPass();
    }
}