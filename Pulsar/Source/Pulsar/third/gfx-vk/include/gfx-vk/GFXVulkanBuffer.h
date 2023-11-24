#pragma once
#include <gfx/GFXBuffer.h>
#include <vulkan/vulkan.h>

namespace gfx
{
    class GFXVulkanApplication;

    class GFXVulkanBuffer : public GFXBuffer
    {
        using base = GFXBuffer;
    public:
        GFXVulkanBuffer(GFXVulkanApplication* app, GFXBufferUsage usage, size_t bufferSize);
        virtual ~GFXVulkanBuffer() override;
    public:
        virtual void Fill(const void* data) override;
        virtual void Release() override;
        const VkBuffer& GetVkBuffer() const { return m_vkBuffer; }
        VkBufferUsageFlags GetVkUsage() const;
        bool IsGpuLocalMemory() const;
        GFXVulkanApplication* GetApplication() const { return m_app; }
    public:
        /* GFXBuffer */
        virtual bool IsValid() const override;
        virtual size_t GetSize() const override { return this->m_bufferSize; }
    public:

    protected:
        bool m_hasData = true;
        VkBuffer m_vkBuffer{};
        VkDeviceMemory m_vkBufferMemory{};
        GFXVulkanApplication* m_app = nullptr;
    };
}