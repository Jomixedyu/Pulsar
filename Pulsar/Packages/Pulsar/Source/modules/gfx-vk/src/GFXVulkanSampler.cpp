#include "GFXVulkanSampler.h"

#include <gfx-vk/BufferHelper.h>
#include <gfx-vk/GFXVulkanApplication.h>

namespace gfx
{
    GFXVulkanSampler::GFXVulkanSampler(GFXVulkanApplication* app, const GFXSamplerConfig& config)
        : base(config), m_app(app)
    {
        auto filter = BufferHelper::GetVkFilter(config.Filter);
        auto addressMode = BufferHelper::GetVkAddressMode(config.AddressMode);
        m_sampler = BufferHelper::CreateTextureSampler(app, filter, addressMode);
    }

    GFXVulkanSampler::~GFXVulkanSampler()
    {
        if (m_sampler)
        {
            vkDestroySampler(m_app->GetVkDevice(), m_sampler, nullptr);
        }
    }
}
