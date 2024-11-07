#pragma once
#include "GFXDescriptorSet.h"
#include "GFXInclude.h"
#include "GFXRenderPass.h"
#include "GFXTextureView.h"

namespace gfx
{
    class GFXFrameBufferObject
    {
    public:
        virtual ~GFXFrameBufferObject() = default;
        virtual int32_t GetWidth() const = 0;
        virtual int32_t GetHeight() const = 0;
        virtual GFXRenderPassLayout_sp GetRenderPassLayout() const = 0;
        virtual const array_list<GFXTexture2DView_sp>& GetRenderTargets() const = 0;

        array_list<GFXDescriptorSet_wp> RefData;
    };
    GFX_DECL_SPTR(GFXFrameBufferObject);
}