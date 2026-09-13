#pragma once
#include "GFXResource.h"
#include "GFXDescriptorSet.h"
#include "GFXRenderPass.h"
#include "GFXTextureView.h"

namespace gfx
{
    class GFXResourceRegistry;

    class GFXFrameBufferObject : public GFXResource
    {
    public:
        ~GFXFrameBufferObject() override = default;
        GFXResourceType GetResourceType() const override { return GFXResourceType::FrameBufferObject; }
        virtual int32_t GetWidth() const = 0;
        virtual int32_t GetHeight() const = 0;
        virtual const GFXRenderTargetDesc& GetRenderTargetDesc() const = 0;
        virtual const array_list<GFXTexture2DViewPtr>& GetRenderTargets() const = 0;

        array_list<GFXDescriptorSet_wp> RefData;

    protected:
        explicit GFXFrameBufferObject(GFXResourceRegistry* registry)
            : GFXResource(registry) {}

    };
    GFX_DECL_PTR(GFXFrameBufferObject);
}
