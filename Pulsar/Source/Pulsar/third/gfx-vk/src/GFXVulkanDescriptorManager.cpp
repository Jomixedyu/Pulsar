#include <gfx-vk/GFXVulkanDescriptorManager.h>
#include <gfx-vk/GFXVulkanApplication.h>

namespace gfx
{
    GFXVulkanDescriptorManager::GFXVulkanDescriptorManager(GFXVulkanApplication* app)
        : m_app(app)
    {
        m_externPool = std::unique_ptr<GFXVulkanDescriptorPool>(new GFXVulkanDescriptorPool(m_app, 512));
    }

    GFXVulkanDescriptorManager::~GFXVulkanDescriptorManager()
    {
        for (auto& pool : m_pools)
        {
            vkResetDescriptorPool(m_app->GetVkDevice(), pool->GetVkDescriptorPool(), 0);
        }
        m_pools.clear();
        m_externPool.reset();
    }

    std::shared_ptr<GFXDescriptorSet> GFXVulkanDescriptorManager::GetDescriptorSet(GFXDescriptorSetLayout_sp layout)
    {
        for (auto& pool : m_pools)
        {
            if (auto ptr = pool->GetDescriptorSet(layout))
            {
                return ptr;
            }
        }

        m_pools.push_back(std::unique_ptr<GFXVulkanDescriptorPool>(new GFXVulkanDescriptorPool(m_app)));

        return m_pools[m_pools.size() - 1]->GetDescriptorSet(layout);
    }
}