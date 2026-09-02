#pragma once
#include "GFXRenderContext.h"
#include "GFXFrameBufferObject.h"
#include <vector>

namespace gfx
{
    class GFXRenderPipeline
    {
    public:
        // Record this frame's GPU commands into the context's command buffers. This does NOT execute
        // on the GPU; execution happens at submit time in the renderer.
        virtual void OnRecord(GFXRenderContext* context, GFXFrameBufferObject* backbuffers) = 0;
        virtual ~GFXRenderPipeline() = default;
    };
}