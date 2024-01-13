#include "GFXVulkanGraphicsPipeline.h"
#include "GFXVulkanApplication.h"
#include "GFXVulkanDescriptorSet.h"
#include "GFXVulkanShaderPass.h"
#include "GFXVulkanVertexLayoutDescription.h"

namespace gfx
{
    static VkPrimitiveTopology _GetVkTopology(GFXPrimitiveTopology topology)
    {
        switch (topology)
        {
        case gfx::GFXPrimitiveTopology::TriangleList:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            break;
        case gfx::GFXPrimitiveTopology::LineList:
            return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
            break;
        default:
            assert(("not implement", false));
            break;
        }
        return {};
    }
    static VkPolygonMode _GetVkPolyMode(GFXPrimitiveTopology topology)
    {
        switch (topology)
        {
        case gfx::GFXPrimitiveTopology::TriangleList:
            return VK_POLYGON_MODE_FILL;
            break;
        case gfx::GFXPrimitiveTopology::LineList:
            return VK_POLYGON_MODE_FILL;
            break;
        default:
            assert(false);
            break;
        }
        return {};
    }

    static VkCompareOp _GetCampareOp(GFXCompareMode mode)
    {
        switch (mode)
        {
        case GFXCompareMode::Never:
            return VK_COMPARE_OP_NEVER;
            break;
        case GFXCompareMode::Less:
            return VK_COMPARE_OP_LESS;
            break;
        case GFXCompareMode::Equal:
            return VK_COMPARE_OP_EQUAL;
            break;
        case GFXCompareMode::LessOrEqual:
            return VK_COMPARE_OP_LESS_OR_EQUAL;
            break;
        case GFXCompareMode::Greater:
            return VK_COMPARE_OP_GREATER;
            break;
        case GFXCompareMode::NotEqual:
            return VK_COMPARE_OP_NOT_EQUAL;
            break;
        case GFXCompareMode::GreaterOrEqual:
            return VK_COMPARE_OP_GREATER_OR_EQUAL;
            break;
        case GFXCompareMode::Always:
            return VK_COMPARE_OP_ALWAYS;
            break;
        default:
            assert(false);
        }
        return {};
    }

    GFXVulkanGraphicsPipeline::GFXVulkanGraphicsPipeline(
        GFXVulkanApplication* app,
        const std::shared_ptr<GFXShaderPass>& shaderPass,
        const array_list<GFXDescriptorSetLayout_sp>& descriptorSetLayouts,
        const GFXRenderPassLayout& renderLayout,
        const GFXGraphicsPipelineState& gpInfo)
        : m_app(app)
    {
        // create pipeline layout
        auto vkShaderPass = std::static_pointer_cast<GFXVulkanShaderPass>(shaderPass);
        auto& vkRenderpass = static_cast<const GFXVulkanRenderPass&>(renderLayout);
        array_list<VkDescriptorSetLayout> vkdescriptorSetLayouts;
        for (const auto& layout : descriptorSetLayouts)
        {
            vkdescriptorSetLayouts.push_back(static_cast<GFXVulkanDescriptorSetLayout*>(layout.get())->GetVkDescriptorSetLayout());
        }

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        // pipelineLayoutInfo.setLayoutCount   = 1;
        pipelineLayoutInfo.setLayoutCount = descriptorSetLayouts.size();
        pipelineLayoutInfo.pSetLayouts = vkdescriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 0;

        if (vkCreatePipelineLayout(app->GetVkDevice(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create pipeline layout!");
        }

        // create vertex info
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        auto& vertLayouts = gpInfo.VertexLayouts;

        array_list<VkVertexInputBindingDescription> bindingDescriptions;
        array_list<VkVertexInputAttributeDescription> attributeDescriptions;

        for (auto& vertLayout : vertLayouts)
        {
            auto vertBinding = std::static_pointer_cast<gfx::GFXVulkanVertexLayoutDescription>(vertLayout);
            bindingDescriptions.push_back(vertBinding->GetVkBindingDescription());
        }
        attributeDescriptions = gfx::GFXVulkanVertexLayoutDescription::GetCombinedAttributes(vertLayouts);

        vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
        vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = _GetVkTopology(gpInfo.Topology);
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        // make pipeline shader state
        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.polygonMode = _GetVkPolyMode(gpInfo.Topology);
        rasterizer.lineWidth = gpInfo.LineWidth;
        rasterizer.cullMode = static_cast<VkCullModeFlagBits>(vkShaderPass->GetStateConfig().CullMode);
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;

        array_list<VkPipelineColorBlendAttachmentState> colorBlendAttachments;

        for (size_t i = 0; i < vkRenderpass.GetColorAttachmentCount(); ++i)
        {
            auto& attachment = colorBlendAttachments.emplace_back();
            attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            attachment.blendEnable = VK_FALSE;
        }

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY;
        colorBlending.attachmentCount = (uint32_t)colorBlendAttachments.size();
        colorBlending.pAttachments = colorBlendAttachments.data();
        colorBlending.blendConstants[0] = 0.0f;
        colorBlending.blendConstants[1] = 0.0f;
        colorBlending.blendConstants[2] = 0.0f;
        colorBlending.blendConstants[3] = 0.0f;

        VkPipelineDepthStencilStateCreateInfo depthStencil{};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = vkShaderPass->GetStateConfig().DepthTestEnable;
        depthStencil.depthWriteEnable = vkShaderPass->GetStateConfig().DepthWriteEnable;
        depthStencil.depthCompareOp = _GetCampareOp(vkShaderPass->GetStateConfig().DepthCompareOp);
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.minDepthBounds = 0.0f; // Optional
        depthStencil.maxDepthBounds = 1.0f; // Optional
        depthStencil.stencilTestEnable = vkShaderPass->GetStateConfig().StencilTestEnable;
        depthStencil.front = {}; // Optional
        depthStencil.back = {};  // Optional

        // other info
        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        std::vector<VkDynamicState> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR,
            VK_DYNAMIC_STATE_CULL_MODE
        };

        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = vkShaderPass->GetVkStages().size();
        pipelineInfo.pStages = vkShaderPass->GetVkStages().data();
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = m_pipelineLayout;
        pipelineInfo.renderPass = static_cast<const GFXVulkanRenderPass&>(renderLayout).GetVkRenderPass();
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
        pipelineInfo.pDepthStencilState = &depthStencil;

        if (vkCreateGraphicsPipelines(app->GetVkDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_pipeline) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create graphics pipeline!");
        }
    }

    GFXVulkanGraphicsPipeline::~GFXVulkanGraphicsPipeline()
    {
        vkDestroyPipelineLayout(m_app->GetVkDevice(), m_pipelineLayout, nullptr);
        vkDestroyPipeline(m_app->GetVkDevice(), m_pipeline, nullptr);
    }
} // namespace gfx