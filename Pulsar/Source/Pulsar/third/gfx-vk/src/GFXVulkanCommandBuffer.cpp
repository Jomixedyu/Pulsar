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
        for (auto rt : m_fbo->GetRenderTargets())
        {
            auto vkrt = static_cast<GFXVulkanRenderTarget*>(rt);

            vkrt->m_imageLayout = vkrt->m_imageFinalLayout;
        }
        vkCmdEndRenderPass(m_cmdBuffer);
    }

    void GFXVulkanCommandBuffer::CmdSetViewport(float x, float y, float width, float height)
    {
        VkViewport viewport{};
#if VULKAN_FLIPPING_VIEWPORT
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

    void GFXVulkanCommandBuffer::CmdBlit(GFXTexture* src, GFXTexture* dest)
    {
        auto _src = static_cast<GFXVulkanRenderTarget*>(src);
        auto _dest = static_cast<GFXVulkanRenderTarget*>(dest);

        VkBlitImageInfo2 info{};
        info.sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2;
        info.srcImage = _src->GetVkImage();
        info.srcImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        info.dstImage = _dest->GetVkImage();
        info.dstImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        info.regionCount = 1;
        VkImageBlit2 regions{};
        regions.sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2;
        regions.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        regions.srcSubresource.baseArrayLayer = 0;
        regions.srcSubresource.layerCount = 1;
        regions.srcOffsets[0] = {0, 0, 0};
        regions.srcOffsets[1] = {_src->GetWidth(), _src->GetHeight(), 1};

        regions.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        regions.dstSubresource.baseArrayLayer = 0;
        regions.dstSubresource.layerCount = 1;
        regions.dstOffsets[0] = {0, 0, 0};
        regions.dstOffsets[1] = {_dest->GetWidth(), _dest->GetHeight(), 1};

        info.pRegions = &regions;
        info.filter = VK_FILTER_LINEAR;
        vkCmdBlitImage2(m_cmdBuffer, &info);
    }

    void GFXVulkanCommandBuffer::CmdImageTransitionBarrier(GFXRenderTarget* rt, GFXResourceLayout layout)
    {
        auto vkrt = static_cast<GFXVulkanRenderTarget*>(rt);
        VkImageLayout newLayout{};
        if (rt->GetRenderTargetType() == GFXRenderTargetType::Color)
        {
            switch (layout)
            {
            case GFXResourceLayout::RenderTarget:
                newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;;
                break;
            case GFXResourceLayout::ShaderReadOnly:
                newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                break;
            }
        }
        else
        {
            switch (layout)
            {
            case GFXResourceLayout::RenderTarget:
                newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                break;
            case GFXResourceLayout::ShaderReadOnly:
                newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
                break;
            }
        }

        vkrt->CmdLayoutTransition(m_cmdBuffer, newLayout);
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

    static VkIndexType _GetVkIndexType(size_t size)
    {
        switch (size)
        {
        case 2:
            return VK_INDEX_TYPE_UINT16;
        case 4:
            return VK_INDEX_TYPE_UINT32;
        default:
            break;
        }
        return {};
    }

    void GFXVulkanCommandBuffer::CmdBindIndexBuffer(GFXBuffer* buffer)
    {
        vkCmdBindIndexBuffer(m_cmdBuffer,
            static_cast<GFXVulkanBuffer*>(buffer)->GetVkBuffer(),
            0,
            _GetVkIndexType(buffer->GetElementSize()));
    }

    void GFXVulkanCommandBuffer::CmdBindDescriptorSets(const array_list<GFXDescriptorSet*>& descriptorSet, GFXGraphicsPipeline* pipeline)
    {
        VkDescriptorSet sets[8]{};
        for (int i = 0; const auto set : descriptorSet)
        {
            const auto vkDescSet = static_cast<GFXVulkanDescriptorSet*>(set);
            sets[i] = vkDescSet->GetVkDescriptorSet();
            ++i;
        }

        const auto vkGpipeline = static_cast<GFXVulkanGraphicsPipeline*>(pipeline);
        vkCmdBindDescriptorSets(m_cmdBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            vkGpipeline->GetVkPipelineLayout(),
            0,
            descriptorSet.size(),
            sets,
            0,
            nullptr);
    }

    void GFXVulkanCommandBuffer::CmdDrawIndexed(size_t indicesCount)
    {
        vkCmdDrawIndexed(m_cmdBuffer, static_cast<uint32_t>(indicesCount), 1, 0, 0, 0);
    }

    void GFXVulkanCommandBuffer::CmdDraw(size_t vertexCount)
    {
        vkCmdDraw(m_cmdBuffer, vertexCount, 1, 0, 0);
    }

    void GFXVulkanCommandBuffer::CmdClearColor(GFXRenderTarget* _rt, float r, float g, float b, float a)
    {
        auto rt = static_cast<GFXVulkanRenderTarget*>(_rt);

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

            auto oldlayout = rt->GetVkImageLayout();
            rt->CmdLayoutTransition(m_cmdBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
            vkCmdClearColorImage(m_cmdBuffer, image, clearLayout, &color, 1, &srRange);
            if (oldlayout == VK_IMAGE_LAYOUT_UNDEFINED)
            {
                rt->CmdLayoutTransition(m_cmdBuffer, rt->GetVkImageFinalLayout());
            }
            else
            {
                rt->CmdLayoutTransition(m_cmdBuffer, oldlayout);
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
    void GFXVulkanCommandBuffer::CmdClearColor(GFXRenderTarget* rt)
    {
        CmdClearColor(rt, rt->ClearColor[0], rt->ClearColor[1], rt->ClearColor[2], rt->ClearColor[3]);
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