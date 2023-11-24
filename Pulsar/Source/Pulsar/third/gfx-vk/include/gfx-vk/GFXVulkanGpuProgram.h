#pragma once
#include "VulkanInclude.h"
#include <gfx/GFXGpuProgram.h>
#include <vector>
#include <cstdint>
#include <unordered_map>
#include <gfx/GFXShaderPass.h>
#include <map>

namespace gfx
{
    class GFXVulkanApplication;

    class GFXVulkanGpuProgram : public GFXGpuProgram
    {
        using base = GFXGpuProgram;
    public:

        GFXVulkanGpuProgram(GFXVulkanApplication* app, const std::unordered_map<gfx::GFXShaderStageFlags, array_list<char>>& codes);
        GFXVulkanGpuProgram(const GFXVulkanGpuProgram&) = delete;
        GFXVulkanGpuProgram(GFXVulkanGpuProgram&&) = delete;
        virtual ~GFXVulkanGpuProgram() override;

    public:
        const VkShaderModule& GetVkShaderModule(GFXShaderStageFlags stage) const
        {
            return m_shaderModules.at(stage);
        }
        const VkShaderModule& GetVkVertShaderModule() const { return GetVkShaderModule(GFXShaderStageFlags::Vertex); }
        const VkShaderModule& GetVkPixelShaderModule() const { return GetVkShaderModule(GFXShaderStageFlags::Fragment); }
    protected:
        std::unordered_map<GFXShaderStageFlags, VkShaderModule> m_shaderModules;

        GFXVulkanApplication* m_app;
    };
    GFX_DECL_SPTR(GFXVulkanGpuProgram);
}