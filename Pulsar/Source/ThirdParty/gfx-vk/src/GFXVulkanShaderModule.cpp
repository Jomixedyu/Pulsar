#include <gfx-vk/GFXVulkanShaderModule.h>
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

    GFXVulkanShaderModule::GFXVulkanShaderModule(GFXVulkanApplication* app, const std::vector<uint8_t>& vert, const std::vector<uint8_t>& frag)
        : base(vert, frag), m_app(app)
    {
        VertShaderModule = _CreateShaderModule(app, vert.data(), vert.size());
        FragShaderModule = _CreateShaderModule(app, frag.data(), frag.size());

        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = VertShaderModule;
        vertShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = FragShaderModule;
        fragShaderStageInfo.pName = "main";

        ShaderStages[0] = vertShaderStageInfo;
        ShaderStages[1] = fragShaderStageInfo;
    }
    GFXVulkanShaderModule::~GFXVulkanShaderModule()
    {
        vkDestroyShaderModule(m_app->GetVkDevice(), FragShaderModule, nullptr);
        vkDestroyShaderModule(m_app->GetVkDevice(), VertShaderModule, nullptr);
    }
}