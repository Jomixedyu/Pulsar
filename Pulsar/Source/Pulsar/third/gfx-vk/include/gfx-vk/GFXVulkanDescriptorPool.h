#pragma once
#include <gfx/GFXDescriptorSet.h>
#include "GFXVulkanDescriptorSet.h"
namespace gfx
{
    class GFXVulkanApplication;

    class GFXVulkanDescriptorPool
    {

    public:
        GFXVulkanDescriptorPool(GFXVulkanApplication* app, size_t maxSetCount = 128);
        ~GFXVulkanDescriptorPool();
    public:
        std::shared_ptr<GFXVulkanDescriptorSet> GetDescriptorSet(const GFXDescriptorSetLayout_sp& layout);
        void ReleaseDescriptorSet();
    public:
        const VkDescriptorPool& GetVkDescriptorPool() const { return m_descriptorPool; }
        GFXVulkanApplication* GetApplication() const { return m_app; }
    protected:
        GFXVulkanApplication* m_app;
        VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
        size_t m_maxSetCount;
        size_t m_count = 0;
    };
}