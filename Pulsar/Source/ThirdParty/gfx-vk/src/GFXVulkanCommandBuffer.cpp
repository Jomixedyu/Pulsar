#include "GFXVulkanCommandBuffer.h"
#include "GFXVulkanCommandBuffer.h"
#include "GFXVulkanApplication.h"
#include "GFXVulkanCommandBufferPool.h"
#include "GFXVulkanShaderPass.h"
#include "GFXVulkanGraphicsPipeline.h"

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

    void GFXVulkanCommandBuffer::CmdEndFrameBuffer()
    {
        vkCmdEndRenderPass(m_cmdBuffer);
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

    void GFXVulkanCommandBuffer::SetFrameBuffer(GFXFrameBufferObject* framebuffer)
    {
        m_fbo = static_cast<GFXVulkanFrameBufferObject*>(framebuffer);
    }

    void GFXVulkanCommandBuffer::CmdBindGraphicsPipeline(GFXGraphicsPipeline* pipeline)
    {
        auto vkpipeline = static_cast<GFXVulkanGraphicsPipeline*>(pipeline);
        vkCmdBindPipeline(m_cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkpipeline->GetVkPipeline());
    }

    void GFXVulkanCommandBuffer::CmdBindVertexBuffers(const std::vector<GFXBuffer*>& buffers)
    {
        std::vector<VkBuffer> vkbuffers;
        for (auto& item : buffers)
        {
            vkbuffers.push_back(static_cast<GFXVulkanBuffer*>(item)->GetVkBuffer());
        }
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(m_cmdBuffer, 0, static_cast<uint32_t>(buffers.size()), vkbuffers.data(), offsets);
    }

    void GFXVulkanCommandBuffer::CmdBindIndexBuffer(GFXBuffer* buffer)
    {
        vkCmdBindIndexBuffer(m_cmdBuffer, static_cast<GFXVulkanBuffer*>(buffer)->GetVkBuffer(), 0, VK_INDEX_TYPE_UINT16);
    }

    void GFXVulkanCommandBuffer::CmdBindDescriptorSets(GFXDescriptorSet* descriptorSet, GFXGraphicsPipeline* pipeline)
    {
        auto vkDescSet = static_cast<GFXVulkanDescriptorSet*>(descriptorSet);
        auto vkGpipeline = static_cast<GFXVulkanGraphicsPipeline*>(pipeline);
        vkCmdBindDescriptorSets(m_cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkGpipeline->GetVkPipelineLayout(), 0, 1, &vkDescSet->GetVkDescriptorSet(), 0, nullptr);
    }

    void GFXVulkanCommandBuffer::CmdDrawIndexed(size_t indicesCount)
    {
        vkCmdDrawIndexed(m_cmdBuffer, static_cast<uint32_t>(indicesCount), 1, 0, 0, 0);
    }

    void GFXVulkanCommandBuffer::CmdClearColor(float r, float g, float b, float a)
    {
        for (auto& rt : m_fbo->GetRenderTargets())
        {
            auto type = rt->GetRenderTargetType();
            auto image = rt->GetVkImage();
            VkImageLayout clearLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

            switch (type)
            {
            case gfx::GFXRenderTargetType::Color:
            {
                VkClearColorValue color{ r,g,b,a };
                VkImageSubresourceRange srRange{};
                srRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                srRange.baseMipLevel = 0;
                srRange.levelCount = 1;
                srRange.baseArrayLayer = 0;
                srRange.layerCount = 1;

                {
                    VkImageMemoryBarrier barrier{};
                    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                    barrier.newLayout = clearLayout;
                    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                    barrier.image = rt->GetVkImage();
                    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    barrier.subresourceRange.baseMipLevel = 0;
                    barrier.subresourceRange.levelCount = 1;
                    barrier.subresourceRange.baseArrayLayer = 0;
                    barrier.subresourceRange.layerCount = 1;

                    vkCmdPipelineBarrier(
                        m_cmdBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                        0, 0, nullptr, 0, nullptr, 1, &barrier);
                }


                vkCmdClearColorImage(m_cmdBuffer, image, clearLayout, &color, 1, &srRange);
                {
                    VkImageMemoryBarrier barrier{};
                    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                    barrier.oldLayout = clearLayout;
                    barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
                    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                    barrier.image = image;
                    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    barrier.subresourceRange.baseMipLevel = 0;
                    barrier.subresourceRange.levelCount = 1;
                    barrier.subresourceRange.baseArrayLayer = 0;
                    barrier.subresourceRange.layerCount = 1;

                    vkCmdPipelineBarrier(
                        m_cmdBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                        0, 0, nullptr, 0, nullptr, 1, &barrier);
                }
                break;
            }
            case gfx::GFXRenderTargetType::DepthStencil:
            case gfx::GFXRenderTargetType::Depth:
            {
                VkImageAspectFlagBits aspectMask = (VkImageAspectFlagBits)rt->GetAspectFlags();
                {
                    VkImageMemoryBarrier barrier{};
                    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                    barrier.newLayout = clearLayout;
                    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                    barrier.image = image;
                    barrier.subresourceRange.aspectMask = aspectMask;
                    barrier.subresourceRange.baseMipLevel = 0;
                    barrier.subresourceRange.levelCount = 1;
                    barrier.subresourceRange.baseArrayLayer = 0;
                    barrier.subresourceRange.layerCount = 1;
                    
                    vkCmdPipelineBarrier(
                        m_cmdBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                        0, 0, nullptr, 0, nullptr, 1, &barrier);
                }

                VkClearDepthStencilValue depthClearValue{};
                depthClearValue.depth = 1;
                depthClearValue.stencil = 0;
                
                VkImageSubresourceRange srRange2{};
                srRange2.aspectMask = aspectMask;
                srRange2.baseMipLevel = 0;
                srRange2.levelCount = 1;
                srRange2.baseArrayLayer = 0;
                srRange2.layerCount = 1;

                vkCmdClearDepthStencilImage(
                    m_cmdBuffer, image,
                    clearLayout, &depthClearValue, 1, &srRange2);

                {
                    VkImageMemoryBarrier barrier{};
                    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                    barrier.oldLayout = clearLayout;
                    barrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                    barrier.image = image;
                    barrier.subresourceRange.aspectMask = aspectMask;
                    barrier.subresourceRange.baseMipLevel = 0;
                    barrier.subresourceRange.levelCount = 1;
                    barrier.subresourceRange.baseArrayLayer = 0;
                    barrier.subresourceRange.layerCount = 1;

                    vkCmdPipelineBarrier(
                        m_cmdBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                        0, 0, nullptr, 0, nullptr, 1, &barrier);
                }

                break;
            }
            default:
                assert(false);
                break;
            }
        }

    }

    void GFXVulkanCommandBuffer::CmdBeginFrameBuffer()
    {
        bool isClear = false;

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_fbo->GetVkRenderPass();
        renderPassInfo.framebuffer = m_fbo->GetVkFrameBuffer();
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = m_fbo->GetVkExtent();

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
        clearValues[1].depthStencil = { 1.0f, 0 };

        renderPassInfo.clearValueCount = isClear ? static_cast<uint32_t>(clearValues.size()) : 0;
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(m_cmdBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
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