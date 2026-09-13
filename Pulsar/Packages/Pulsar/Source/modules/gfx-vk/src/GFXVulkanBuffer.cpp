#include <gfx-vk/GFXVulkanBuffer.h>
#include <gfx-vk/GFXVulkanApplication.h>
#include <gfx-vk/GFXVulkanResourceRegistry.h>
#include <gfx-vk/BufferHelper.h>
#include <gfx/GFXInclude.h>
#include <cassert>
#include <stdexcept>

namespace gfx
{

    static VkMemoryPropertyFlags ToVkBufferProperties(GFXBufferMemoryPosition pos)
    {
        switch (pos)
        {
        case GFXBufferMemoryPosition::VisibleOnHost:
            return VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        case GFXBufferMemoryPosition::VisibleOnDevice:
            return VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        case GFXBufferMemoryPosition::DeviceLocal:
            return VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        };
        assert(false);
        return {};
    }
    GFXVulkanBuffer::GFXVulkanBuffer(GFXResourceRegistry* registry, const GFXBufferDesc& desc)
        : GFXBuffer(registry, desc)
    {
        auto* app = GetApplication();
        auto vkUsage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | GetVkUsage();
        auto memoryProperty = ToVkBufferProperties(desc.StorageType);
        BufferHelper::CreateBuffer(app, desc.BufferSize, vkUsage, memoryProperty, m_vkBuffer, m_vkBufferMemory);
    }

    GFXVulkanApplication* GFXVulkanBuffer::GetApplication() const
    {
        auto* registry = static_cast<GFXVulkanResourceRegistry*>(GetResourceRegistry());
        return registry ? registry->GetVulkanApplication() : nullptr;
    }

    GFXVulkanBuffer::~GFXVulkanBuffer()
    {
        auto* app = GetApplication();
        if (m_hasData)
        {
            BufferHelper::DestroyBuffer(app, m_vkBuffer, m_vkBufferMemory);
            m_hasData = false;
        }

        m_vkBuffer = {};
        m_vkBufferMemory = {};
    }



    void GFXVulkanBuffer::Update(const void* data)
    {
        auto* app = GetApplication();
        if (IsDeviceLocal())
        {
            // Device-local memory can't be mapped; upload through a transient staging buffer.
            VkBuffer stagingBuffer;
            VkDeviceMemory stagingBufferMemory;
            BufferHelper::CreateBuffer(app, m_desc.BufferSize,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                stagingBuffer, stagingBufferMemory
            );

            void* memData;
            vkMapMemory(app->GetVkDevice(), stagingBufferMemory, 0, m_desc.BufferSize, 0, &memData);
            memcpy(memData, data, m_desc.BufferSize);
            vkUnmapMemory(app->GetVkDevice(), stagingBufferMemory);

            BufferHelper::TransferBuffer(app, stagingBuffer, m_vkBuffer, m_desc.BufferSize);
            BufferHelper::DestroyBuffer(app, stagingBuffer, stagingBufferMemory);
        }
        else
        {
            void* gpuData;
            vkMapMemory(app->GetVkDevice(), m_vkBufferMemory, 0, m_desc.BufferSize, 0, &gpuData);
            memcpy(gpuData, data, m_desc.BufferSize);
            vkUnmapMemory(app->GetVkDevice(), m_vkBufferMemory);
        }
    }

    VkBufferUsageFlags GFXVulkanBuffer::GetVkUsage() const
    {
        VkBufferUsageFlags vkUsage = 0;
        if (HasFlag(m_desc.Usage, GFXBufferUsage::Vertex))
            vkUsage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        if (HasFlag(m_desc.Usage, GFXBufferUsage::Indices))
            vkUsage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        if (HasFlag(m_desc.Usage, GFXBufferUsage::ConstantBuffer))
            vkUsage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        if (HasFlag(m_desc.Usage, GFXBufferUsage::StructuredBuffer))
            vkUsage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        assert(vkUsage != 0 && "buffer usage not specified");
        return vkUsage;
    }

    bool GFXVulkanBuffer::IsValid() const
    {
        return m_hasData;
    }
}
