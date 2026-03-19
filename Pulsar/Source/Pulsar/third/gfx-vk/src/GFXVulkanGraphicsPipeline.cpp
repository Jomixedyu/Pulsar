#include "GFXVulkanGraphicsPipeline.h"
#include "GFXVulkanApplication.h"
#include "GFXVulkanDescriptorSet.h"
#include "GFXVulkanGpuProgram.h"
#include "GFXVulkanVertexLayoutDescription.h"
#include "BufferHelper.h"

namespace gfx
{
    static VkPrimitiveTopology _GetVkTopology(GFXPrimitiveTopology topology)
    {
        switch (topology)
        {
        case GFXPrimitiveTopology::TriangleList:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        case GFXPrimitiveTopology::LineList:
            return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        }
        assert(("not implement", false));
        return {};
    }

    static VkPolygonMode _GetVkPolyMode(GFXPrimitiveTopology topology)
    {
        switch (topology)
        {
        case GFXPrimitiveTopology::TriangleList:
            return VK_POLYGON_MODE_FILL;
        case GFXPrimitiveTopology::LineList:
            return VK_POLYGON_MODE_FILL;
        }
        assert(false);
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
        const array_list<GFXGpuProgram_sp>& gpuPrograms,
        GFXGraphicsPipelineStateParams stateParams,
        const array_list<GFXDescriptorSetLayout_sp>& descriptorSetLayouts,
        const GFXRenderTargetDesc& renderTargetDesc,
        const GFXGraphicsPipelineState& gpInfo)
        : m_app(app)
    {
        // create pipeline layout
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
        rasterizer.cullMode = static_cast<VkCullModeFlagBits>(stateParams.CullMode);
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;

        array_list<VkPipelineColorBlendAttachmentState> colorBlendAttachments;

        for (size_t i = 0; i < renderTargetDesc.ColorFormats.size(); ++i)
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
        depthStencil.depthTestEnable = stateParams.DepthTestEnable;
        depthStencil.depthWriteEnable = stateParams.DepthWriteEnable;
        depthStencil.depthCompareOp = _GetCampareOp(stateParams.DepthCompareOp);
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.minDepthBounds = 0.0f; // Optional
        depthStencil.maxDepthBounds = 1.0f; // Optional
        depthStencil.stencilTestEnable = stateParams.StencilTestEnable;
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

        VkPipelineShaderStageCreateInfo shaderStage[6];
        int shaderStageCount = 0;
        for (auto& gpuProgram : gpuPrograms)
        {
            auto vkGpuProgram = static_cast<GFXVulkanGpuProgram*>(gpuProgram.get());
            shaderStage[shaderStageCount] = vkGpuProgram->GetCreateInfo();
            shaderStageCount++;
        }
        pipelineInfo.stageCount = shaderStageCount;
        pipelineInfo.pStages = shaderStage;

        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = m_pipelineLayout;
        pipelineInfo.renderPass = VK_NULL_HANDLE;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
        pipelineInfo.pDepthStencilState = &depthStencil;

        // Dynamic Rendering: VkPipelineRenderingCreateInfo
        array_list<VkFormat> vkColorFormats;
        for (auto fmt : renderTargetDesc.ColorFormats)
        {
            vkColorFormats.push_back(BufferHelper::GetVkFormat(fmt));
        }

        VkPipelineRenderingCreateInfo pipelineRenderingInfo{};
        pipelineRenderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
        pipelineRenderingInfo.colorAttachmentCount = static_cast<uint32_t>(vkColorFormats.size());
        pipelineRenderingInfo.pColorAttachmentFormats = vkColorFormats.data();
        VkFormat depthStencilVkFormat = renderTargetDesc.HasDepthStencil
            ? BufferHelper::GetVkFormat(renderTargetDesc.DepthStencilFormat)
            : VK_FORMAT_UNDEFINED;
        pipelineRenderingInfo.depthAttachmentFormat = depthStencilVkFormat;

        // If the depth format also contains a stencil component, set stencilAttachmentFormat
        bool hasStencil = (depthStencilVkFormat == VK_FORMAT_D32_SFLOAT_S8_UINT ||
                           depthStencilVkFormat == VK_FORMAT_D24_UNORM_S8_UINT ||
                           depthStencilVkFormat == VK_FORMAT_D16_UNORM_S8_UINT);
        pipelineRenderingInfo.stencilAttachmentFormat = hasStencil ? depthStencilVkFormat : VK_FORMAT_UNDEFINED;

        pipelineInfo.pNext = &pipelineRenderingInfo;

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