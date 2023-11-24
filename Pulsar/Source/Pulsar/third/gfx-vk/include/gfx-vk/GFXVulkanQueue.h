#pragma once
#include <gfx/GFXQueue.h>
#include "VulkanInclude.h"

namespace gfx
{
    class GFXVulkanApplication;

    class GFXVulkanQueue : public GFXQueue
    {
    public:
        GFXVulkanQueue(GFXVulkanApplication* app, VkSemaphore waitSemaphore, VkSemaphore signalSemaphore, VkFence fence);
    public:
        virtual void Submit(const std::vector<const GFXCommandBuffer*>& buffer);
        void VkSubmit(class GFXVulkanCommandBuffer* buffers, size_t count);
    public:
        const VkFence& GetVkFence() const { return m_fence; }
        const VkSemaphore& GetVkWaitSemaphore() const { return m_waitSemaphore; }
        const VkSemaphore& GetVkSignalSemaphore() const { return m_signalSemaphore; }

    private:
        void Internal_Submit(VkCommandBuffer* buffer, size_t len);
    protected:
        VkFence m_fence;
        VkSemaphore m_signalSemaphore;
        VkSemaphore m_waitSemaphore;
        GFXVulkanApplication* m_app;
    };
}