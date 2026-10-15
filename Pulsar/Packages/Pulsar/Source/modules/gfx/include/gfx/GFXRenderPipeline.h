#pragma once
#include "GFXCommandBuffer.h"
#include "GFXFrameBufferObject.h"

namespace gfx
{
    class GFXRenderPipeline
    {
    public:
        // Record this frame's GPU commands into cmd. The command buffer is begun by the renderer
        // before this call and is ended and submitted after it returns. Recording here does NOT
        // execute anything on the GPU; execution happens at submit time in the renderer.
        virtual void OnRecord(GFXCommandBuffer& cmd, GFXFrameBufferObject* backbuffers) = 0;
        virtual ~GFXRenderPipeline() = default;
    };
}