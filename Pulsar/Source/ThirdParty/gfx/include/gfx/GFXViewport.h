#pragma once
#include "GFXRenderTarget.h"
#include "GFXFrameBufferObject.h"

namespace gfx
{
    class GFXViewport
    {
    public:
        virtual GFXFrameBufferObject* GetFrameBufferObject() = 0;
    public:
        virtual ~GFXViewport() {}
    };
}