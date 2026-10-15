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
    class GFXResourceRegistry;

    class GFXVulkanGpuProgram : public GFXGpuProgram
    {
        using base = GFXGpuProgram;
    public:

        GFXVulkanGpuProgram(GFXResourceRegistry* registry = nullptr, GFXGpuProgramStageFlags stage = {}, const uint8_t* codes = nullptr, size_t len = 0);
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

    private:
        GFXVulkanApplication* GetApplication() const;
    };
    GFX_DECL_PTR(GFXVulkanGpuProgram);
}
