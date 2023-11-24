#include <gfx-vk/GFXVulkanGpuProgram.h>
#include <gfx-vk/GFXVulkanApplication.h>
#include <stdexcept>

namespace gfx
{
    VkShaderModule _CreateShaderModule(GFXVulkanApplication* app, const char* code, size_t len)
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
        GFXVulkanApplication* app, 
        const std::unordered_map<gfx::GFXShaderStageFlags, array_list<char>>& codes)
        : m_app(app)
    {
        for (auto& [stage, buf] : codes)
        {
            m_shaderModules.insert({ stage, _CreateShaderModule(m_app, buf.data(), buf.size()) });
        }
    }
    GFXVulkanGpuProgram::~GFXVulkanGpuProgram()
    {
        for (auto& [stage, sModule] : m_shaderModules)
        {
            vkDestroyShaderModule(m_app->GetVkDevice(), sModule, nullptr);
        }
        m_shaderModules.clear();
    }
}