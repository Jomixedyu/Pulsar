#pragma once
#include "VulkanInclude.h"
#include <vector>
#include <queue>

namespace gfx
{
    class GFXVulkanApplication;

    class GFXVulkanCommandBufferPool
    {
    public:
        GFXVulkanCommandBufferPool(GFXVulkanApplication* app);
        ~GFXVulkanCommandBufferPool();

    public:
        VkCommandBuffer GetVkCommandBuffer();
    public:
        const VkCommandPool& GetVkCommandPool() const { return m_pool; }

        void ReleaseCommandBuffer(VkCommandBuffer buffer);
    private:
        void AllocCommandBuffer();
    protected:
        GFXVulkanApplication* m_app;
        VkCommandPool m_pool;

        std::queue<VkCommandBuffer> m_initial;
    };
}