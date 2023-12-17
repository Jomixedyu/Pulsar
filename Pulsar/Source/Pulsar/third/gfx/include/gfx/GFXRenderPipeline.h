#pragma once
#include "GFXRenderContext.h"
#include "GFXFrameBufferObject.h"
#include <vector>

namespace gfx
{
    class GFXRenderPipeline
    {
    public:
        virtual void OnRender(GFXRenderContext* context, GFXFrameBufferObject* backbuffers) = 0;
        virtual ~GFXRenderPipeline() = default;
    };
}