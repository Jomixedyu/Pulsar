#pragma once
#include "GFXFrameBufferObject.h"

namespace gfx
{
    class GFXSwapchain
    {
    public:
        virtual GFXFrameBufferObject* GetFrameBufferObject() = 0;
        virtual void SetSize(int width, int height) = 0;
        virtual void GetSize(int* width, int* height) const = 0;
    public:
        virtual ~GFXSwapchain() {}
    };
}