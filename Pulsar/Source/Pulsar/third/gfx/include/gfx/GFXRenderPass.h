#pragma once
#include "GFXInclude.h"

namespace gfx
{
    class GFXRenderPassLayout
    {
    public:
        virtual ~GFXRenderPassLayout() = default;
        GFXRenderPassLayout() = default;
        GFXRenderPassLayout(const GFXRenderPassLayout&) = delete;
        GFXRenderPassLayout(GFXRenderPassLayout&&) = delete;
    };
    GFX_DECL_SPTR(GFXRenderPassLayout);
}