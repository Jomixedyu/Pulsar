#pragma once
#include <gfx/GFXDescriptorManager.h>
#include "GFXVulkanDescriptorPool.h"
#include <memory>

namespace gfx
{
    class GFXVulkanApplication;
    class GFXVulkanDescriptorManager : public GFXDescriptorManager
    {
    public:
        GFXVulkanDescriptorManager(GFXVulkanApplication* app);
        virtual ~GFXVulkanDescriptorManager() override;

        virtual std::shared_ptr<GFXDescriptorSet> GetDescriptorSet(GFXDescriptorSetLayout_sp layout) override;
        
        GFXVulkanDescriptorPool* GetCommonDescriptorSetPool() const { return m_externPool.get(); }
    protected:
        GFXVulkanApplication* m_app;
        std::vector<std::unique_ptr<GFXVulkanDescriptorPool>> m_pools;

        std::unique_ptr<GFXVulkanDescriptorPool> m_externPool;
    };
}

