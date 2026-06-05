#pragma once
#include <functional>

namespace gfx
{
    class GFXRenderer
    {
    public:
        virtual ~GFXRenderer()
        {
        }

        virtual void WaitExecuteRender(const std::function<void(GFXRenderContext*)>& func) = 0;
    };
}