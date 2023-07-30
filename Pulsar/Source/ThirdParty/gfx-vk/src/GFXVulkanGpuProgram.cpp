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

    GFXVulkanGpuProgram::GFXVulkanGpuProgram(GFXVulkanApplication* app, const std::vector<uint8_t>& vert, const std::vector<uint8_t>& frag)
        : base(vert, frag), m_app(app)
    {
        m_vertShaderModule = _CreateShaderModule(app, vert.data(), vert.size());
        m_pixelShaderModule = _CreateShaderModule(app, frag.data(), frag.size());
    }
    GFXVulkanGpuProgram::~GFXVulkanGpuProgram()
    {
        vkDestroyShaderModule(m_app->GetVkDevice(), m_vertShaderModule, nullptr);
        vkDestroyShaderModule(m_app->GetVkDevice(), m_pixelShaderModule, nullptr);
    }
}