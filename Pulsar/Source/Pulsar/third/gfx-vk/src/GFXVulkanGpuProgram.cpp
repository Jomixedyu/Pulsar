#include <gfx-vk/GFXVulkanGpuProgram.h>
#include <gfx-vk/GFXVulkanApplication.h>
#include <stdexcept>

namespace gfx
{
    VkShaderModule _CreateShaderModule(GFXVulkanApplication* app, const uint8_t* code, size_t len)
    {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = len;
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code);

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(app->GetVkDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create shader module!");
        }

        return shaderModule;
    }

    GFXVulkanGpuProgram::GFXVulkanGpuProgram(
        GFXVulkanApplication* app, GFXGpuProgramStageFlags stage, const uint8_t* codes, size_t len)
        : m_app(app), m_stage(stage)
    {
        m_shader = _CreateShaderModule(m_app, codes, len);
    }
    GFXVulkanGpuProgram::~GFXVulkanGpuProgram()
    {
        vkDestroyShaderModule(m_app->GetVkDevice(), m_shader, nullptr);
    }
    VkShaderStageFlagBits GFXVulkanGpuProgram::GetVkStage() const
    {
        switch (m_stage)
        {
        case GFXGpuProgramStageFlags::Vertex: return VK_SHADER_STAGE_VERTEX_BIT;
        case GFXGpuProgramStageFlags::Fragment: return VK_SHADER_STAGE_FRAGMENT_BIT;
        case GFXGpuProgramStageFlags::Compute: return VK_SHADER_STAGE_COMPUTE_BIT;
        case GFXGpuProgramStageFlags::TessellationControl: return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        case GFXGpuProgramStageFlags::TessellationEvaluation: return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
        case GFXGpuProgramStageFlags::Geometry: return VK_SHADER_STAGE_GEOMETRY_BIT;
        case GFXGpuProgramStageFlags::Mesh: return VK_SHADER_STAGE_MESH_BIT_EXT;

        case GFXGpuProgramStageFlags::VertexFragment: return VkShaderStageFlagBits(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
        }
        assert(false);
        return {};
    }
    VkPipelineShaderStageCreateInfo GFXVulkanGpuProgram::GetCreateInfo() const
    {
        VkPipelineShaderStageCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        info.pName = m_entryName.c_str();
        info.module = m_shader;
        info.stage = GetVkStage();
        return info;
    }
} // namespace gfx