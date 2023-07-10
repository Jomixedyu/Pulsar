#pragma once
#include "VulkanInclude.h"
#include <gfx/GFXShaderModule.h>
#include <vector>
#include <cstdint>

namespace gfx
{
    class GFXVulkanApplication;

    struct GFXVulkanShaderModule : public GFXShaderModule
    {
        using base = GFXShaderModule;
    public:

        GFXVulkanShaderModule(GFXVulkanApplication* app, const std::vector<uint8_t>& vert, const std::vector<uint8_t>& frag);
        GFXVulkanShaderModule(const GFXVulkanShaderModule&) = delete;
        GFXVulkanShaderModule(GFXVulkanShaderModule&&) = delete;
        ~GFXVulkanShaderModule();

        VkPipelineShaderStageCreateInfo ShaderStages[2]{};

        VkShaderModule VertShaderModule;
        VkShaderModule FragShaderModule;
    protected:
        GFXVulkanApplication* m_app;
    };
}