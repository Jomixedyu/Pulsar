#include <gfx-vk/GFXVulkanDescriptorPool.h>
#include <gfx-vk/GFXVulkanApplication.h>
#include <gfx-vk/VulkanInclude.h>
#include <gfx-vk/GFXVulkanDescriptorSet.h>
#include <unordered_map>
#include <stdexcept>
#include <cassert>

namespace gfx
{
    static std::unordered_map<VkDescriptorType, float> DefaultPoolSizes
    {
        { VkDescriptorType::VK_DESCRIPTOR_TYPE_SAMPLER, 2 },
        { VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2 },
        { VkDescriptorType::VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 2 },
        { VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1 / 8.0f },
        { VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1 / 2.0f },
        { VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1 / 8.0f },
        { VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 / 4.0f },
        { VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1 / 8.0f },
        { VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 4 },
        { VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1 / 8.0f },
        { VkDescriptorType::VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1 / 8.0f },
    };

    GFXVulkanDescriptorPool::GFXVulkanDescriptorPool(GFXVulkanApplication* app, size_t maxSetCount)
        : m_app(app), m_maxSetCount(maxSetCount)
    {
        std::vector<VkDescriptorPoolSize> poolSizes;

        for (auto& item : DefaultPoolSizes)
        {
            VkDescriptorPoolSize poolSize;
            poolSize.type = item.first;
            poolSize.descriptorCount = static_cast<size_t>(maxSetCount * item.second);
            poolSizes.push_back(poolSize);
        }

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = static_cast<uint32_t>(maxSetCount);
        poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

        if (vkCreateDescriptorPool(app->GetVkDevice(), &poolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create descriptor pool!");
        }
    }
    GFXVulkanDescriptorPool::~GFXVulkanDescriptorPool()
    {
        vkDestroyDescriptorPool(m_app->GetVkDevice(), m_descriptorPool, nullptr);
    }

    std::shared_ptr<GFXVulkanDescriptorSet> GFXVulkanDescriptorPool::GetDescriptorSet(const GFXDescriptorSetLayout_sp& layout)
    {
        if (m_count == m_maxSetCount)
        {
            return nullptr;
        }

        ++m_count;
        return std::shared_ptr<GFXVulkanDescriptorSet>{ new GFXVulkanDescriptorSet(this, layout) };
    }

    void GFXVulkanDescriptorPool::ReleaseDescriptorSet()
    {
        --m_count;
    }
}