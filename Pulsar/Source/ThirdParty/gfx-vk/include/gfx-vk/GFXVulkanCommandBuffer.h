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
        virtual void SetFrameBuffer(GFXFrameBufferObject* framebuffer) override;

        virtual void CmdBindGraphicsPipeline(GFXGraphicsPipeline* pipeline) override;
        virtual void CmdBindVertexBuffers(const std::vector<GFXBuffer*>& buffers) override;
        virtual void CmdBindIndexBuffer(GFXBuffer* buffer) override;
        virtual void CmdBindDescriptorSets(GFXDescriptorSet* descriptorSet, GFXGraphicsPipeline* pipeline) override;
        virtual void CmdDrawIndexed(size_t indicesCount) override;
        virtual void CmdClearColor(float r, float g, float b, float a) override;
        virtual void CmdBeginFrameBuffer() override;
        virtual void CmdEndFrameBuffer() override;
        virtual void CmdSetViewport(float x, float y, float width, float height) override;

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