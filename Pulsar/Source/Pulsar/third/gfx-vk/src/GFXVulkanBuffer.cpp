#include <gfx-vk/GFXVulkanBuffer.h>
#include <gfx-vk/GFXVulkanApplication.h>
#include <gfx-vk/BufferHelper.h>
#include <cassert>
#include <stdexcept>

namespace gfx
{

    GFXVulkanBuffer::GFXVulkanBuffer(GFXVulkanApplication* app, GFXBufferUsage usage, size_t bufferSize)
        : m_app(app), base(usage, bufferSize)
    {

        if (IsGpuLocalMemory())
        {
            BufferHelper::CreateBuffer(m_app, m_bufferSize,
                VK_BUFFER_USAGE_TRANSFER_DST_BIT | GetVkUsage(),
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                m_vkBuffer, m_vkBufferMemory
            );
        }
        else
        {
            BufferHelper::CreateBuffer(m_app, m_bufferSize,
                GetVkUsage(),
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                m_vkBuffer, m_vkBufferMemory);
        }

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
            BufferHelper::CreateBuffer(m_app, m_bufferSize,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                stagingBuffer, stagingBufferMemory
            );

            void* memData;
            vkMapMemory(m_app->GetVkDevice(), stagingBufferMemory, 0, m_bufferSize, 0, &memData);
            memcpy(memData, data, m_bufferSize);
            vkUnmapMemory(m_app->GetVkDevice(), stagingBufferMemory);

            //transfer
            BufferHelper::TransferBuffer(m_app, stagingBuffer, m_vkBuffer, m_bufferSize);
            BufferHelper::DestroyBuffer(m_app, stagingBuffer, stagingBufferMemory);
        }
        else
        {
            void* gpuData;
            vkMapMemory(m_app->GetVkDevice(), m_vkBufferMemory, 0, m_bufferSize, 0, &gpuData);
            memcpy(gpuData, data, m_bufferSize);
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
        switch (m_usage)
        {
        case gfx::GFXBufferUsage::Vertex:
            vkUsage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            break;
        case gfx::GFXBufferUsage::Index:
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
        return m_usage != GFXBufferUsage::ConstantBuffer && m_usage != GFXBufferUsage::StructuredBuffer;
    }

    bool GFXVulkanBuffer::IsValid() const
    {
        return m_hasData;
    }
}