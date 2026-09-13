#pragma once
#include "VulkanInclude.h"

#include <gfx/GFXSampler.h>

namespace gfx
{
    class GFXVulkanApplication;
    class GFXResourceRegistry;

    class GFXVulkanSampler : public GFXSampler
    {
        using base = GFXSampler;
    public:
        GFXVulkanSampler(GFXResourceRegistry* registry = nullptr, const GFXSamplerConfig& config = {});
        ~GFXVulkanSampler() override;

        GFXVulkanSampler(const GFXVulkanSampler&) = delete;

        VkSampler GetVkSampler() const { return m_sampler; }

    private:
        GFXVulkanApplication* GetApplication() const;
        VkSampler m_sampler{};
    };
}
