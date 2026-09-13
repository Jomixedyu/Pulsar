#pragma once
#include <gfx/GFXBuffer.h>
#include <vulkan/vulkan.h>

namespace gfx
{
    class GFXVulkanApplication;
    class GFXResourceRegistry;

    class GFXVulkanBuffer : public GFXBuffer
    {
        using base = GFXBuffer;
    public:
        GFXVulkanBuffer(GFXResourceRegistry* registry = nullptr, const GFXBufferDesc& desc = {});
        virtual ~GFXVulkanBuffer() override;
    public:
        virtual void Update(const void* data) override;
        const VkBuffer& GetVkBuffer() const { return m_vkBuffer; }
        VkBufferUsageFlags GetVkUsage() const;
    public:
        /* GFXBuffer */
        virtual bool IsValid() const override;
        virtual size_t GetSize() const override { return this->m_desc.BufferSize; }
    public:

    protected:
        bool m_hasData = true;

    private:
        GFXVulkanApplication* GetApplication() const;
        VkBuffer m_vkBuffer{};
        VkDeviceMemory m_vkBufferMemory{};
    };
}
