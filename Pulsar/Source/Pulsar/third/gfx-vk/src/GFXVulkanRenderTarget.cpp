#include "GFXVulkanRenderTarget.h"
#include "GFXVulkanApplication.h"
#include "BufferHelper.h"
#include <stacktrace>
#include <queue>

struct DestroyedInsight
{
    static inline std::vector<DestroyedInsight*> instances;

    struct Info
    {
        std::string FileInfo;
        std::string CreateStack;
        void* Ptr;
    };

    std::vector<Info> Infos;
    std::string DefinePosition;
    const char* Name;

public:
    DestroyedInsight(const char* name, const std::string& definePosition) :
        Name(name), DefinePosition(definePosition)
    {
        instances.push_back(this);
    }

    void Add(const std::string& fileInfo, void* ptr)
    {
        Infos.push_back({ fileInfo, std::to_string(std::stacktrace::current(1)), ptr });
    }
    void Dec(void* ptr)
    {
        auto it = std::find_if(Infos.begin(), Infos.end(), [&](const Info& info) { return info.Ptr == ptr; });
        if (it != Infos.end())
        {
            Infos.erase(it);
        }
        else
        {
            assert(0);
        }
    }
};
#define DESTROYED_INSIGHT_DEFINE(Name) static DestroyedInsight Name(#Name, std::string(__FILE__) + std::to_string(__LINE__));
#define DESTROYED_INSIGHT_CTOR(Name) Name.Add(std::string(__FILE__) + std::to_string(__LINE__), this);
#define DESTROYED_INSIGHT_DESTOR(Name) Name.Dec(this);

namespace gfx
{
    DESTROYED_INSIGHT_DEFINE(vkRtDOI);

    GFXVulkanRenderTarget::GFXVulkanRenderTarget(
        int32_t width, int32_t height,
        VkImage image, VkImageView imageView, VkFormat format, VkImageLayout layout, GFXRenderTargetType type)
        : m_width(width), m_height(height), m_textureImage(image), m_textureImageView(imageView),
        m_imageFormat(format), m_imageFinalLayout(layout), m_imageLayout(VK_IMAGE_LAYOUT_UNDEFINED),
        m_type(type), m_mode(CreateMode_View)
    {
        //DESTROYED_INSIGHT_CTOR(vkRtDOI);

    }

    GFXVulkanRenderTarget::GFXVulkanRenderTarget(
        GFXVulkanApplication* app, int32_t width, int32_t height,
        GFXRenderTargetType type, GFXTextureFormat format, const GFXSamplerConfig& samplerCfg)
        : m_app(app), m_type(type), m_mode(CreateMode_New),
        m_width(width), m_height(height), m_aspectFlags(0)
    {
        m_imageFormat = BufferHelper::GetVkFormat(format);

        VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT
            | VK_IMAGE_USAGE_TRANSFER_DST_BIT
            | VK_IMAGE_USAGE_SAMPLED_BIT;

        if (type == gfx::GFXRenderTargetType::Color)
        {
            m_imageFinalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
            m_aspectFlags |= VK_IMAGE_ASPECT_COLOR_BIT;
        }
        else if (type == gfx::GFXRenderTargetType::DepthStencil)
        {
            m_imageFinalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
            m_aspectFlags |= VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        }
        else
        {
            assert(0);
        }

        BufferHelper::CreateImage(app, width, height,
            m_imageFormat,
            VK_IMAGE_TILING_OPTIMAL, usage,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_textureImage, m_textureImageMemory);

        BufferHelper::TransitionImageLayout(app, m_textureImage, m_imageFormat,
            VK_IMAGE_LAYOUT_UNDEFINED, m_imageFinalLayout);
        m_imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        m_textureImageView = BufferHelper::CreateImageView(m_app, m_textureImage, m_imageFormat, m_aspectFlags);
        m_textureSampler = BufferHelper::CreateTextureSampler(m_app);

        //DESTROYED_INSIGHT_CTOR(vkRtDOI);
    }
    GFXVulkanRenderTarget::~GFXVulkanRenderTarget()
    {
        if (m_mode & CreateMode_ShouldDeleted)
        {
            vkDestroyImage(m_app->GetVkDevice(), m_textureImage, nullptr);
            vkFreeMemory(m_app->GetVkDevice(), m_textureImageMemory, nullptr);
            vkDestroyImageView(m_app->GetVkDevice(), m_textureImageView, nullptr);
            if (m_textureSampler)
            {
                vkDestroySampler(m_app->GetVkDevice(), m_textureSampler, nullptr);
            }
        }
        // DESTROYED_INSIGHT_DESTOR(vkRtDOI);
    }

    void GFXVulkanRenderTarget::CmdLayoutTransition(VkCommandBuffer cmd, VkImageLayout newLayout)
    {
        BufferHelper::TransitionImageLayout(cmd, m_textureImage, m_imageFormat, m_imageLayout, newLayout);
        m_imageLayout = newLayout;
    }
} // namespace gfx