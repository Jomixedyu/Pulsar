#include "GFXVulkanSampler.h"

#include <gfx-vk/BufferHelper.h>
#include <gfx-vk/GFXVulkanApplication.h>
#include <gfx-vk/GFXVulkanResourceRegistry.h>

namespace gfx
{
    GFXVulkanSampler::GFXVulkanSampler(GFXResourceRegistry* registry, const GFXSamplerConfig& config)
        : GFXSampler(registry, config)
    {
        auto* app = GetApplication();
        auto filter = BufferHelper::GetVkFilter(config.Filter);
        auto addressMode = BufferHelper::GetVkAddressMode(config.AddressMode);
        m_sampler = BufferHelper::CreateTextureSampler(app, filter, addressMode);
    }

    GFXVulkanApplication* GFXVulkanSampler::GetApplication() const
    {
        auto* registry = static_cast<GFXVulkanResourceRegistry*>(GetResourceRegistry());
        return registry ? registry->GetVulkanApplication() : nullptr;
    }

    GFXVulkanSampler::~GFXVulkanSampler()
    {
        if (m_sampler)
        {
            if (auto* app = GetApplication())
            {
                vkDestroySampler(app->GetVkDevice(), m_sampler, nullptr);
            }
            m_sampler = VK_NULL_HANDLE;
        }
    }
}
