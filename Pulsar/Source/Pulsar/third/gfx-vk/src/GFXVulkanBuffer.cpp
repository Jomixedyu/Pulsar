#include <gfx-vk/GFXVulkanBuffer.h>
#include <gfx-vk/GFXVulkanApplication.h>
#include <gfx-vk/BufferHelper.h>
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
    GFXVulkanBuffer::GFXVulkanBuffer(GFXVulkanApplication* app, const GFXBufferDesc& desc)
        : m_app(app), base(desc)
    {
        auto vkUsage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | GetVkUsage();
        auto memoryProperty = ToVkBufferProperties(desc.StorageType);
        BufferHelper::CreateBuffer(m_app, desc.BufferSize, vkUsage, memoryProperty, m_vkBuffer, m_vkBufferMemory);
    }

    GFXVulkanBuffer::~GFXVulkanBuffer()
    {
        if (m_hasData)
        {
            BufferHelper::DestroyBuffer(m_app, m_vkBuffer, m_vkBufferMemory);
            m_hasData = false;
        }
        // if (this->IsValid())
        // {
        //     this->Release();
        // }
    }



    void GFXVulkanBuffer::Fill(const void* data)
    {
        if (IsGpuLocalMemory())
        {
            //create staging buffer
            VkBuffer stagingBuffer;
            VkDeviceMemory stagingBufferMemory;
            BufferHelper::CreateBuffer(m_app, m_desc.BufferSize,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                stagingBuffer, stagingBufferMemory
            );

            void* memData;
            vkMapMemory(m_app->GetVkDevice(), stagingBufferMemory, 0, m_desc.BufferSize, 0, &memData);
            memcpy(memData, data, m_desc.BufferSize);
            vkUnmapMemory(m_app->GetVkDevice(), stagingBufferMemory);

            //transfer
            BufferHelper::TransferBuffer(m_app, stagingBuffer, m_vkBuffer, m_desc.BufferSize);
            BufferHelper::DestroyBuffer(m_app, stagingBuffer, stagingBufferMemory);
        }
        else
        {
            void* gpuData;
            vkMapMemory(m_app->GetVkDevice(), m_vkBufferMemory, 0, m_desc.BufferSize, 0, &gpuData);
            memcpy(gpuData, data, m_desc.BufferSize);
            vkUnmapMemory(m_app->GetVkDevice(), m_vkBufferMemory);
        }

    }

    void GFXVulkanBuffer::Release()
    {
        if (m_hasData)
        {
            BufferHelper::DestroyBuffer(m_app, m_vkBuffer, m_vkBufferMemory);
            m_hasData = false;
        }
    }

    VkBufferUsageFlags GFXVulkanBuffer::GetVkUsage() const
    {
        VkBufferUsageFlags vkUsage;
        switch (m_desc.Usage)
        {
        case gfx::GFXBufferUsage::Vertex:
            vkUsage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            break;
        case gfx::GFXBufferUsage::Indices:
            vkUsage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
            break;
        case GFXBufferUsage::ConstantBuffer:
            vkUsage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
            break;
        case GFXBufferUsage::StructuredBuffer:
            vkUsage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
            break;
        default:
            assert(false);
            break;
        }
        return vkUsage;
    }

    bool GFXVulkanBuffer::IsGpuLocalMemory() const
    {
        return m_desc.Usage != GFXBufferUsage::ConstantBuffer && m_desc.Usage != GFXBufferUsage::StructuredBuffer;
    }

    bool GFXVulkanBuffer::IsValid() const
    {
        return m_hasData;
    }
}