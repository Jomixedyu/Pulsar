#pragma once
#include <gfx/GFXCommandBuffer.h>
#include "VulkanInclude.h"
#include "GFXVulkanFrameBufferObject.h"
#include "GFXVulkanBuffer.h"
#include "GFXVulkanDescriptorSet.h"
#include "GFXVulkanShaderPass.h"
#include <array>

namespace gfx
{
    class GFXVulkanApplication;

    class GFXVulkanCommandBuffer : public GFXCommandBuffer
    {

    public:
        GFXVulkanCommandBuffer(GFXVulkanApplication* app);
        GFXVulkanCommandBuffer(GFXVulkanCommandBuffer&& r) noexcept;

        virtual ~GFXVulkanCommandBuffer() override;

        virtual void Begin() override;
        virtual void End() override;


        virtual void CmdBindShaderPass(GFXShaderPass* pipeline);
        virtual void CmdBindVertexBuffers(const std::vector<GFXBuffer*>& buffers)
        {
            std::vector<VkBuffer> vkbuffers;
            for (auto& item : buffers)
            {
                vkbuffers.push_back(static_cast<GFXVulkanBuffer*>(item)->GetVkBuffer());
            }
            VkDeviceSize offsets[] = { 0 };
            vkCmdBindVertexBuffers(m_cmdBuffer, 0, buffers.size(), vkbuffers.data(), offsets);
        }
        void CmdBindIndexBuffer(GFXBuffer* buffer)
        {
            vkCmdBindIndexBuffer(m_cmdBuffer, static_cast<GFXVulkanBuffer*>(buffer)->GetVkBuffer(), 0, VK_INDEX_TYPE_UINT16);
        }
        void CmdBindDescriptorSets(GFXDescriptorSet* descriptorSet, GFXShaderPass* shaderPass)
        {
            auto vkDescSet = static_cast<GFXVulkanDescriptorSet*>(descriptorSet);
            auto vkShaderPass = static_cast<GFXVulkanShaderPass*>(shaderPass);
            vkCmdBindDescriptorSets(m_cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkShaderPass->GetVkPipelineLayout(), 0, 1, &vkDescSet->GetVkDescriptorSet(), 0, nullptr);
        }
        void CmdDrawIndexed(size_t indicesCount)
        {
            vkCmdDrawIndexed(m_cmdBuffer, static_cast<uint32_t>(indicesCount), 1, 0, 0, 0);
        }
        void SetFrameBuffer(GFXVulkanFrameBufferObject* framebuffer)
        {
            m_fbo = framebuffer;
        }



        void CmdClearColor(float r, float g, float b, float a)
        {
            for (auto& rt : m_fbo->GetRenderTargets())
            {
                auto type = rt->GetRenderTargetType();
                auto image = rt->GetVulkanTexture2d()->GetVkImage();
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
                        barrier.image = rt->GetVulkanTexture2d()->GetVkImage();
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
                case gfx::GFXRenderTargetType::Depth:
                {
                    {
                        VkImageMemoryBarrier barrier{};
                        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                        barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                        barrier.newLayout = clearLayout;
                        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                        barrier.image = image;
                        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
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
                    srRange2.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
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
                        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
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
        void CmdBeginFrameBuffer()
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

        void CmdEndFrameBuffer()
        {
            vkCmdEndRenderPass(m_cmdBuffer);
        }
        void CmdSetViewport(float x, float y, float width, float height);

    public:
        virtual GFXApplication* GetApplication() const override;
        const VkCommandBuffer& GetVkCommandBuffer() const { return m_cmdBuffer; }
    protected:
        VkCommandBuffer m_cmdBuffer = VK_NULL_HANDLE;
        GFXVulkanApplication* m_app;
        GFXVulkanFrameBufferObject* m_fbo = nullptr;
    public:

    };


    struct GFXVulkanCommandBufferScope
    {
    private:
        GFXVulkanApplication* m_app;
        std::shared_ptr<GFXVulkanCommandBuffer> m_cmdbuffer;

    public:
        GFXVulkanCommandBufferScope(GFXVulkanApplication* app);

        GFXCommandBuffer* operator->() const
        {
            return m_cmdbuffer.get();
        }
        ~GFXVulkanCommandBufferScope();

        GFXVulkanCommandBufferScope(const GFXVulkanCommandBufferScope&) = delete;
        GFXVulkanCommandBufferScope(GFXVulkanCommandBufferScope&&) = delete;
    };
}