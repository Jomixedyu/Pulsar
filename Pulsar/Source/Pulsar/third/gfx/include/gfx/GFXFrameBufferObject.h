#pragma once
#include "GFXResource.h"
#include "GFXDescriptorSet.h"
#include "GFXRenderPass.h"
#include "GFXTextureView.h"

namespace gfx
{
    class GFXFrameBufferObject : public GFXResource
    {
    public:
        ~GFXFrameBufferObject() override = default;
        GFXResourceType GetResourceType() const override { return GFXResourceType::FrameBufferObject; }
        virtual int32_t GetWidth() const = 0;
        virtual int32_t GetHeight() const = 0;
        virtual const GFXRenderTargetDesc& GetRenderTargetDesc() const = 0;
        virtual const array_list<GFXTexture2DView_sp>& GetRenderTargets() const = 0;

        array_list<GFXDescriptorSet_wp> RefData;
    };
    GFX_DECL_SPTR(GFXFrameBufferObject);
}