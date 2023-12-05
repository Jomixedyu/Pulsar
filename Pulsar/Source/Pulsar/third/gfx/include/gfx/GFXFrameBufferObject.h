#pragma once
#include "GFXInclude.h"
#include "GFXRenderTarget.h"
#include "GFXRenderPass.h"
#include "GFXDescriptorSet.h"

namespace gfx
{
    class GFXFrameBufferObject
    {
    public:
        virtual ~GFXFrameBufferObject() {}
        virtual int32_t GetWidth() const = 0;
        virtual int32_t GetHeight() const = 0;
        virtual GFXRenderPassLayout_sp GetRenderPassLayout() const = 0;
        virtual const array_list<GFXRenderTarget*>& GetRenderTargets() const = 0;

        array_list<GFXDescriptorSet_wp> RefData;
    };
    GFX_DECL_SPTR(GFXFrameBufferObject);
}