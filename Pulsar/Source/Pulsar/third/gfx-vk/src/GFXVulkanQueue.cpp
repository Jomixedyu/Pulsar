#include "GFXVulkanQueue.h"
#include "GFXVulkanCommandBuffer.h"
#include "GFXVulkanApplication.h"

namespace gfx
{
    GFXVulkanQueue::GFXVulkanQueue(GFXVulkanApplication* app, VkSemaphore waitSemaphore, VkSemaphore signalSemaphore, VkFence fence)
        : m_app(app), m_waitSemaphore(waitSemaphore), m_signalSemaphore(signalSemaphore), m_fence(fence)
    {
        //VkFenceCreateInfo info{};
        //info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        //info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        //vkCreateFence(app->GetVkDevice(), &info, nullptr, &m_fence);
    }

    void GFXVulkanQueue::Internal_Submit(VkCommandBuffer* buffer, size_t len)
    {
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = static_cast<uint32_t>(len);
        submitInfo.pCommandBuffers = buffer;

        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

        if (m_waitSemaphore)
        {
            submitInfo.waitSemaphoreCount = 1;
            submitInfo.pWaitSemaphores = &m_waitSemaphore;
            submitInfo.pWaitDstStageMask = waitStages;
        }
        if (m_signalSemaphore)
        {
            submitInfo.signalSemaphoreCount = 1;
            submitInfo.pSignalSemaphores = &m_signalSemaphore;
        }

        vkQueueSubmit(m_app->GetVkGraphicsQueue(), 1, &submitInfo, m_fence);
    }

    void GFXVulkanQueue::Submit(const std::vector<const GFXCommandBuffer*>& buffer)
    {
        std::vector<VkCommandBuffer> vkBuffers;
        vkBuffers.reserve(buffer.size());
        for (auto& item : buffer)
        {
            VkCommandBuffer vkbuf = static_cast<const GFXVulkanCommandBuffer*>(item)->GetVkCommandBuffer();
            vkBuffers.push_back(vkbuf);
        }

        Internal_Submit(vkBuffers.data(), vkBuffers.size());
    }

    void GFXVulkanQueue::VkSubmit(GFXVulkanCommandBuffer* buffers, size_t count)
    {
        std::vector<VkCommandBuffer> vkBuffers;
        vkBuffers.reserve(count);
        for (size_t i = 0; i < count; i++)
        {
            vkBuffers.push_back(buffers[i].GetVkCommandBuffer());
        }

        Internal_Submit(vkBuffers.data(), vkBuffers.size());
    }
}