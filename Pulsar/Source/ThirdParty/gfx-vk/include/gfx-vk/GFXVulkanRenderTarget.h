#pragma once
#include <gfx/GFXRenderTarget.h>
#include "VulkanInclude.h"
#include "GFXVulkanTexture2D.h"

namespace gfx
{
    class GFXVulkanApplication;

    class GFXVulkanRenderTarget : public GFXRenderTarget
    {
    public:
        // create view
        GFXVulkanRenderTarget(GFXVulkanTexture2D* tex, GFXRenderTargetType type)
            : m_app(nullptr), m_tex2d(tex), m_type(type)
        {

        }

        // create
        GFXVulkanRenderTarget(
            GFXVulkanApplication* app, int32_t width, int32_t height, 
            GFXRenderTargetType type, GFXTextureFormat format, const GFXSamplerConfig& samplerCfg);

        GFXVulkanRenderTarget(const GFXVulkanRenderTarget&) = delete;
        GFXVulkanRenderTarget(GFXVulkanRenderTarget&&) = delete;
        virtual ~GFXVulkanRenderTarget() override;

    public:
        GFXVulkanTexture2D* GetVulkanTexture2d() const { return m_tex2d; }
        GFXRenderTargetType GetRenderTargetType() const { return m_type; }
        virtual int32_t GetWidth() const override { return  m_tex2d->GetWidth(); }
        virtual int32_t GetHeight() const override { return m_tex2d->GetHeight(); }
    protected:
        GFXVulkanApplication* m_app;
        GFXRenderTargetType m_type;
        GFXVulkanTexture2D* m_tex2d;

        //int32_t m_width;
        //int32_t m_height;

        //VkImage m_textureImage = VK_NULL_HANDLE;
        //VkDeviceMemory m_textureMemory = VK_NULL_HANDLE;
    };
}