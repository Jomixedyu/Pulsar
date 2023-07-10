#pragma once
#include <gfx/GFXRenderTarget.h>
#include "VulkanInclude.h"
#include "GFXVulkanTexture2D.h"

namespace gfx
{
    class GFXVulkanRenderTarget : public GFXRenderTarget
    {
    public:
        // create view
        GFXVulkanRenderTarget(GFXVulkanTexture2D* tex, GFXRenderTargetType type)
            : m_tex2d(tex), m_type(type)
        {

        }

        GFXVulkanRenderTarget(const GFXVulkanRenderTarget&) = delete;
        GFXVulkanRenderTarget(GFXVulkanRenderTarget&&) = delete;

        GFXVulkanTexture2D* GetVulkanTexture2d() const { return m_tex2d; }
        GFXRenderTargetType GetRenderTargetType() const { return m_type; }
        virtual int32_t GetWidth() const override { return m_tex2d->GetWidth(); }
        virtual int32_t GetHeight() const override { return m_tex2d->GetHeight(); }
    protected:

        GFXRenderTargetType m_type;
        GFXVulkanTexture2D* m_tex2d;
    };
}