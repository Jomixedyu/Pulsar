#pragma once
#include "GFXCommandBuffer.h"
#include <vector>

namespace gfx
{
    class GFXQueue
    {
    public:
        virtual ~GFXQueue() {}
        virtual void Submit(const std::vector<const GFXCommandBuffer*>& commandBuffer) = 0;
    };
}