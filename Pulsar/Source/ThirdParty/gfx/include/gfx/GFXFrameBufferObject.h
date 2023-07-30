#pragma once
#include "GFXInclude.h"
#include "GFXRenderTarget.h"
#include "GFXRenderPass.h"

namespace gfx
{
    class GFXFrameBufferObject
    {
    public:
        virtual ~GFXFrameBufferObject() {}
        virtual int32_t GetWidth() const = 0;
        virtual int32_t GetHeight() const = 0;
        virtual std::shared_ptr<GFXRenderPassLayout> GetRenderPassLayout() const = 0;
    };
}