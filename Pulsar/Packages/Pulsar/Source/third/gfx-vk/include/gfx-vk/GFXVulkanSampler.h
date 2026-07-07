#pragma once
#include "VulkanInclude.h"

#include <gfx/GFXSampler.h>

namespace gfx
{
    class GFXVulkanApplication;

    class GFXVulkanSampler : public GFXSampler
    {
        using base = GFXSampler;
    public:
        GFXVulkanSampler(GFXVulkanApplication* app, const GFXSamplerConfig& config);
        ~GFXVulkanSampler() override;

        GFXVulkanSampler(const GFXVulkanSampler&) = delete;

        VkSampler GetVkSampler() const { return m_sampler; }

    private:
        GFXVulkanApplication* m_app;
        VkSampler m_sampler{};
    };
}
