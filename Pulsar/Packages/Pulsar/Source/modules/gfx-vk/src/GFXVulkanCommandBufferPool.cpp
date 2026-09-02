#include "GFXVulkanCommandBufferPool.h"
#include "GFXVulkanApplication.h"
#include "PhysicalDeviceHelper.h"
#include <cassert>

namespace gfx
{
    GFXVulkanCommandBufferPool::GFXVulkanCommandBufferPool(GFXVulkanApplication* app)
        : m_app(app)
    {
        vk::QueueFamilyIndices queueFamilyIndices = vk::PhysicalDeviceHelper::FindQueueFamilies(app->GetVkSurface(), app->GetVkPhysicalDevice());

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

        if (vkCreateCommandPool(app->GetVkDevice(), &poolInfo, nullptr, &m_pool) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create command pool!");
        }
    }
    GFXVulkanCommandBufferPool::~GFXVulkanCommandBufferPool()
    {
        vkDestroyCommandPool(m_app->GetVkDevice(), m_pool, nullptr);
    }

    VkCommandBuffer GFXVulkanCommandBufferPool::GetVkCommandBuffer()
    {
        if (m_initial.size() == 0)
        {
            AllocCommandBuffer();
        }
        assert(m_initial.size() != 0);

        auto buf = m_initial.front();
        m_initial.pop();
        return buf;
    }
    void GFXVulkanCommandBufferPool::ReleaseCommandBuffer(VkCommandBuffer buffer)
    {
        vkResetCommandBuffer(buffer, 0);
        m_initial.push(buffer);
    }

    void GFXVulkanCommandBufferPool::AllocCommandBuffer()
    {
        const int allocSize = 3;

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = m_pool;
        allocInfo.commandBufferCount = allocSize;
        
        VkCommandBuffer buffers[allocSize]{};

        vkAllocateCommandBuffers(m_app->GetVkDevice(), &allocInfo, buffers);

        for (size_t i = 0; i < allocSize; i++)
        {
            m_initial.push(buffers[i]);
        }
    }

}