#pragma once
#include "VulkanInclude.h"
#include <gfx/GFXGpuProgram.h>
#include <vector>
#include <cstdint>
#include <unordered_map>
#include <map>

namespace gfx
{
    class GFXVulkanApplication;

    class GFXVulkanGpuProgram : public GFXGpuProgram
    {
        using base = GFXGpuProgram;
    public:

        GFXVulkanGpuProgram(GFXVulkanApplication* app, GFXGpuProgramStageFlags stage, const uint8_t* codes, size_t len);
        GFXVulkanGpuProgram(const GFXVulkanGpuProgram&) = delete;
        GFXVulkanGpuProgram(GFXVulkanGpuProgram&&) = delete;
        virtual ~GFXVulkanGpuProgram() override;

    public:

        GFXGpuProgramStageFlags GetStage() const override { return m_stage; }
        VkShaderStageFlagBits GetVkStage() const;
        VkPipelineShaderStageCreateInfo GetCreateInfo() const;

    protected:
        GFXGpuProgramStageFlags m_stage;
        VkShaderModule m_shader;
        GFXVulkanApplication* m_app;
    };
    GFX_DECL_SPTR(GFXVulkanGpuProgram);
}