#pragma once

namespace gfx
{
    class GFXRenderPassLayout
    {
    public:
        virtual ~GFXRenderPassLayout() {}
        GFXRenderPassLayout() {}
        GFXRenderPassLayout(const GFXRenderPassLayout&) = delete;
        GFXRenderPassLayout(GFXRenderPassLayout&&) = delete;
    };
}