#pragma once
#include "GFXRenderContext.h"
#include "GFXFrameBufferObject.h"
#include <vector>

namespace gfx
{
    class GFXRenderPipeline
    {
    public:
        virtual void OnRender(GFXRenderContext* context, const std::vector<GFXFrameBufferObject*>& renderTargets) = 0;
    };
}