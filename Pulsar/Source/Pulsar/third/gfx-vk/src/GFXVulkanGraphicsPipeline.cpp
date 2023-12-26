#include "GFXVulkanGraphicsPipeline.h"
#include "GFXVulkanDescriptorSet.h"
#include "GFXVulkanApplication.h"
#include "GFXVulkanVertexLayoutDescription.h"
#include "GFXVulkanShaderPass.h"

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
            break;
        }
        assert(("not implement", false));
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
            break;
        }
    }

    GFXVulkanGraphicsPipeline::GFXVulkanGraphicsPipeline(
        GFXVulkanApplication* app,
        const std::shared_ptr<GFXShaderPass>& shaderPass,
        const array_list<GFXDescriptorSetLayout_sp>& descriptorSetLayouts,
        const GFXRenderPassLayout& renderLayout,
        const GFXGraphicsPipelineState& gpInfo
    )
        : m_app(app)
    {
        // create pipeline layout
        auto vkShaderPass = std::static_pointer_cast<GFXVulkanShaderPass>(shaderPass);

        array_list<VkDescriptorSetLayout> vkdescriptorSetLayouts;
        for (auto layout : descriptorSetLayouts)
        {
            vkdescriptorSetLayouts.push_back(static_cast<GFXVulkanDescriptorSetLayout*>(layout.get())->GetVkDescriptorSetLayout());
        }

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType            = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        //pipelineLayoutInfo.setLayoutCount   = 1;
        pipelineLayoutInfo.setLayoutCount   = descriptorSetLayouts.size();
        pipelineLayoutInfo.pSetLayouts      = vkdescriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 0;

        if (vkCreatePipelineLayout(app->GetVkDevice(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create pipeline layout!");
        }

        // create vertex info
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        auto vertLayouts = shaderPass->GetVertexLayout();

        array_list<VkVertexInputBindingDescription>   bindingDescriptions;
        array_list<VkVertexInputAttributeDescription> attributeDescriptions;

        for (auto& vertLayout : vertLayouts)
        {
            auto vertBinding = std::static_pointer_cast<gfx::GFXVulkanVertexLayoutDescription>(vertLayout);
            bindingDescriptions.push_back(vertBinding->GetVkBindingDescription());
        }
        attributeDescriptions = gfx::GFXVulkanVertexLayoutDescription::GetCombinedAttributes(vertLayouts);

        vertexInputInfo.vertexBindingDescriptionCount   = static_cast<uint32_t>(bindingDescriptions.size());
        vertexInputInfo.pVertexBindingDescriptions      = bindingDescriptions.data();
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexAttributeDescriptions    = attributeDescriptions.data();


        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType      = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology   = _GetVkTopology(gpInfo.Topology);
        inputAssembly.primitiveRestartEnable = VK_FALSE;


        // make pipeline shader state
        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType             = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable  = VK_FALSE;
        rasterizer.polygonMode       = _GetVkPolyMode(gpInfo.Topology);
        rasterizer.lineWidth         = gpInfo.LineWidth;
        rasterizer.cullMode          = static_cast<VkCullModeFlagBits>(vkShaderPass->GetStateConfig().CullMode);
        rasterizer.frontFace         = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizer.depthBiasEnable   = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;


        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable    = VK_FALSE;

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable     = VK_FALSE;
        colorBlending.logicOp           = VK_LOGIC_OP_COPY;
        colorBlending.attachmentCount   = 1;
        colorBlending.pAttachments      = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f;
        colorBlending.blendConstants[1] = 0.0f;
        colorBlending.blendConstants[2] = 0.0f;
        colorBlending.blendConstants[3] = 0.0f;

        VkPipelineDepthStencilStateCreateInfo depthStencil{};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable   = VK_TRUE;
        depthStencil.depthWriteEnable  = VK_TRUE;
        depthStencil.depthCompareOp    = VK_COMPARE_OP_LESS;
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.minDepthBounds    = 0.0f; // Optional
        depthStencil.maxDepthBounds    = 1.0f; // Optional
        depthStencil.stencilTestEnable = VK_FALSE;
        depthStencil.front = {}; // Optional
        depthStencil.back = {}; // Optional

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
        dynamicState.pDynamicStates    = dynamicStates.data();

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable  = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;


        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount          = vkShaderPass->GetVkStages().size();
        pipelineInfo.pStages             = vkShaderPass->GetVkStages().data();
        pipelineInfo.pVertexInputState   = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState      = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState   = &multisampling;
        pipelineInfo.pColorBlendState    = &colorBlending;
        pipelineInfo.pDynamicState       = &dynamicState;
        pipelineInfo.layout              = m_pipelineLayout;
        pipelineInfo.renderPass          = static_cast<const GFXVulkanRenderPass&>(renderLayout).GetVkRenderPass();
        pipelineInfo.subpass             = 0;
        pipelineInfo.basePipelineHandle  = VK_NULL_HANDLE;
        pipelineInfo.pDepthStencilState  = &depthStencil;

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
}