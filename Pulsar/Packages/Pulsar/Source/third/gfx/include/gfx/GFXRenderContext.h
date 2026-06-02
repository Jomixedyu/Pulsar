#pragma once
#include "GFXCommandBuffer.h"

namespace gfx
{
    class GFXApplication;

    class GFXRenderContext
    {
    public:
        virtual GFXApplication* GetApplication() = 0;

        virtual GFXCommandBuffer& AddCommandBuffer() = 0;
        virtual GFXCommandBuffer& GetCommandBuffer(size_t index) = 0;
        virtual void Submit() = 0;

        float DeltaTime;
    };
}