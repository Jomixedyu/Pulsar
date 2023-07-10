#include "GFXVulkanCommandBuffer.h"
#include "GFXVulkanCommandBuffer.h"
#include "GFXVulkanApplication.h"
#include "GFXVulkanCommandBufferPool.h"
#include "GFXVulkanShaderPass.h"


namespace gfx
{
    GFXVulkanCommandBuffer::GFXVulkanCommandBuffer(GFXVulkanApplication* app)
        : m_app(app)
    {
        m_cmdBuffer = app->GetCommandBufferPool()->GetVkCommandBuffer();
    }
    GFXVulkanCommandBuffer::~GFXVulkanCommandBuffer()
    {
        if (m_cmdBuffer != VK_NULL_HANDLE)
        {
            m_app->GetCommandBufferPool()->ReleaseCommandBuffer(m_cmdBuffer);
            m_cmdBuffer = VK_NULL_HANDLE;
        }
    }
    GFXVulkanCommandBuffer::GFXVulkanCommandBuffer(GFXVulkanCommandBuffer&& r) noexcept
    {
        m_app = r.m_app;
        m_cmdBuffer = r.m_cmdBuffer;
        m_fbo = r.m_fbo;

        r.m_cmdBuffer = VK_NULL_HANDLE;
    }

    void GFXVulkanCommandBuffer::CmdSetViewport(float x, float y, float width, float height)
    {
        VkViewport viewport{};
#if VULKAN_REVERT_VIEWPORT
        viewport.x = x;
        viewport.y = y + height;
        viewport.width = width;
        viewport.height = -height;
#else
        viewport.x = x;
        viewport.y = y;
        viewport.width = width;
        viewport.height = height;
#endif
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        vkCmdSetViewport(m_cmdBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = { (uint32_t)width, (uint32_t)height };
        vkCmdSetScissor(m_cmdBuffer, 0, 1, &scissor);
    }


    GFXApplication* GFXVulkanCommandBuffer::GetApplication() const
    {
        return m_app;
    }
    void GFXVulkanCommandBuffer::Begin()
    {
        vkResetCommandBuffer(m_cmdBuffer, /*VkCommandBufferResetFlagBits*/ 0);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        //beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(m_cmdBuffer, &beginInfo);
    }
    void GFXVulkanCommandBuffer::End()
    {
        vkEndCommandBuffer(m_cmdBuffer);
    }

    void GFXVulkanCommandBuffer::CmdBindShaderPass(GFXShaderPass* pipeline)
    {
        auto vkpipeline = static_cast<GFXVulkanShaderPass*>(pipeline);
        vkCmdBindPipeline(m_cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkpipeline->GetVkPipeline());
    }




    GFXVulkanCommandBufferScope::GFXVulkanCommandBufferScope(GFXVulkanApplication* app)
        : m_app(app)
    {
        m_cmdbuffer = std::static_pointer_cast<GFXVulkanCommandBuffer>(app->CreateCommandBuffer());

        m_cmdbuffer->Begin();
    }

    GFXVulkanCommandBufferScope::~GFXVulkanCommandBufferScope()
    {
        m_cmdbuffer->End();

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &m_cmdbuffer->GetVkCommandBuffer();

        vkQueueSubmit(m_app->GetVkGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(m_app->GetVkGraphicsQueue());

        m_cmdbuffer.reset();
    }

}