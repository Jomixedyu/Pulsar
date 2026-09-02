#pragma once
#include "VulkanInclude.h"
#include "GFXVulkanQueue.h"
#include <gfx/GFXRenderContext.h>
#include "GFXVulkanCommandBuffer.h"
#include <memory>
#include <vector>

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
            m_queue->VkSubmit(m_buffers);
        }
        virtual GFXApplication* GetApplication() override;

        virtual GFXCommandBuffer& AddCommandBuffer() override
        {
            return *m_buffers.emplace_back(std::make_unique<GFXVulkanCommandBuffer>(m_app));
        }
        // Insert a command buffer at the front so it is submitted (and executed) before all others.
        // Used for the frame-head staging->device transfer, which must complete before render passes
        // read the uploaded buffers. Call after OnRecord so index-0 lookups during recording are unaffected.
        GFXVulkanCommandBuffer& PrependCommandBuffer()
        {
            return *m_buffers.emplace(m_buffers.begin(), std::make_unique<GFXVulkanCommandBuffer>(m_app))->get();
        }
        virtual GFXCommandBuffer& GetCommandBuffer(size_t index) override
        {
            return *m_buffers[index];
        }
    public:
        GFXVulkanQueue* GetQueue() const { return m_queue; }
        void SetQueue(GFXVulkanQueue* queue) { m_queue = queue; }
    protected:
        GFXVulkanApplication* m_app;
        GFXVulkanQueue* m_queue;
        std::vector<std::unique_ptr<GFXVulkanCommandBuffer>> m_buffers;
    };
}