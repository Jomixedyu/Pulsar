#include "GFXVulkanRenderPass.h"
#include "GFXVulkanRenderPass.h"
#include "GFXVulkanApplication.h"
#include "BufferHelper.h"

namespace gfx
{
    static VkImageLayout _GetFinalLayout(GFXRenderTargetType type)
    {
        switch (type)
        {
        case gfx::GFXRenderTargetType::Color:
            return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            break;
        case gfx::GFXRenderTargetType::Depth:
            return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            break;
        default:
            assert(("msg: not implemented exception", false));
            break;
        }
        return {};
    }
    static VkImageLayout _GetRefLayout(GFXRenderTargetType type)
    {
        switch (type)
        {
        case gfx::GFXRenderTargetType::Color:
            return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            break;
        case gfx::GFXRenderTargetType::Depth:
            return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            break;
        default:
            assert(false);
            break;
        }
        return {};
    }

    GFXVulkanRenderPass::GFXVulkanRenderPass(GFXVulkanApplication* app, const std::vector<GFXVulkanRenderTarget*>& createFromlayout)
        : m_app(app)
    {
        std::vector<VkAttachmentDescription> attachmentDesc;
        std::vector<VkAttachmentReference> colorAttachmentRef;
        std::vector<VkAttachmentReference> depthAttachmentRef;

        for (size_t i = 0; i < createFromlayout.size(); i++)
        {
            auto& rt = createFromlayout[i];
            auto imageFormat = rt->GetVulkanTexture2d()->GetVkImageFormat();

            VkAttachmentDescription attachment{};
            {
                attachment.format = imageFormat;
                attachment.samples = VK_SAMPLE_COUNT_1_BIT;
                //attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                attachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                attachment.finalLayout = _GetFinalLayout(rt->GetRenderTargetType());
            }

            VkAttachmentReference ref{};
            ref.attachment = static_cast<uint32_t>(i);
            ref.layout = _GetRefLayout(rt->GetRenderTargetType());

            attachmentDesc.push_back(attachment);

            if (rt->GetRenderTargetType() == GFXRenderTargetType::Color)
            {
                colorAttachmentRef.push_back(ref);
            }
            else if (rt->GetRenderTargetType() == GFXRenderTargetType::Depth)
            {
                depthAttachmentRef.push_back(ref);
            }

        }

        VkSubpassDescription subpass{};
        {
            subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpass.colorAttachmentCount = static_cast<uint32_t>(colorAttachmentRef.size());
            subpass.pColorAttachments = colorAttachmentRef.data();
            if (depthAttachmentRef.size())
            {
                subpass.pDepthStencilAttachment = depthAttachmentRef.data();
            }
        }


        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        //dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        VkRenderPassCreateInfo renderPassInfo{};
        {
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            renderPassInfo.attachmentCount = 2;
            renderPassInfo.pAttachments = attachmentDesc.data();
            renderPassInfo.subpassCount = 1;
            renderPassInfo.pSubpasses = &subpass;
            renderPassInfo.dependencyCount = 1;
            renderPassInfo.pDependencies = &dependency;
        }

        if (vkCreateRenderPass(m_app->GetVkDevice(), &renderPassInfo, nullptr, &m_renderPass) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create render pass!");
        }
    }

    GFXVulkanRenderPass::~GFXVulkanRenderPass()
    {
        if (m_renderPass != VK_NULL_HANDLE)
        {
            vkDestroyRenderPass(m_app->GetVkDevice(), m_renderPass, nullptr);
            m_renderPass = VK_NULL_HANDLE;
        }
    }
}