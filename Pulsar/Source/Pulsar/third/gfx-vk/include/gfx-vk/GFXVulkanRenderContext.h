#pragma once
#include "VulkanInclude.h"
#include "GFXVulkanQueue.h"
#include <gfx/GFXRenderContext.h>
#include "GFXVulkanCommandBuffer.h"

namespace gfx
{
    class GFXVulkanApplication;

    class GFXVulkanRenderContext : public GFXRenderContext
    {
    public:
        GFXVulkanRenderContext(GFXVulkanApplication* app)
            : m_app(app)
        {

        }
    public:
        virtual void Submit() override
        {
            //std::vector<const GFXCommandBuffer*> buffer;
            //for (auto& buf : m_buffers)
            //{
            //    buffer.push_back(&buf);
            //}
            //m_queue->Submit(buffer);
            m_queue->VkSubmit(m_buffers.data(), m_buffers.size());
        }
        virtual GFXApplication* GetApplication() override;

        virtual GFXCommandBuffer& AddCommandBuffer() override
        {
            return m_buffers.emplace_back(m_app);
        }
        virtual GFXCommandBuffer& GetCommandBuffer(size_t index) override
        {
            return m_buffers[index];
        }
    public:
        GFXVulkanQueue* GetQueue() const { return m_queue; }
        void SetQueue(GFXVulkanQueue* queue) { m_queue = queue; }
    protected:
        GFXVulkanApplication* m_app;
        GFXVulkanQueue* m_queue;
        std::vector<GFXVulkanCommandBuffer> m_buffers;
    };
}