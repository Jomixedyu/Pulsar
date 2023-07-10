#pragma once
#include <cstdint>
#include "GFXRenderTarget.h"

namespace gfx
{
    class GFXFrameBufferObject
    {
    public:
        virtual ~GFXFrameBufferObject() {}
        virtual int32_t GetWidth() const = 0;
        virtual int32_t GetHeight() const = 0;
    };
}