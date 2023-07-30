#pragma once
#include "VulkanInclude.h"
#include <gfx/GFXGpuProgram.h>
#include <vector>
#include <cstdint>

namespace gfx
{
    class GFXVulkanApplication;

    struct GFXVulkanGpuProgram : public GFXGpuProgram
    {
        using base = GFXGpuProgram;
    public:

        GFXVulkanGpuProgram(GFXVulkanApplication* app, const std::vector<uint8_t>& vert, const std::vector<uint8_t>& frag);
        GFXVulkanGpuProgram(const GFXVulkanGpuProgram&) = delete;
        GFXVulkanGpuProgram(GFXVulkanGpuProgram&&) = delete;
        virtual ~GFXVulkanGpuProgram() override;

    public:
        const VkShaderModule& GetVkVertShaderModule() const { return m_vertShaderModule; }
        const VkShaderModule& GetVkPixelShaderModule() const { return m_pixelShaderModule; }
    protected:
        VkShaderModule m_vertShaderModule;
        VkShaderModule m_pixelShaderModule;
        GFXVulkanApplication* m_app;
    };
}