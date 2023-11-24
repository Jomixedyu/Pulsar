#pragma once
#include "GFXInclude.h"

namespace gfx
{
    class GFXGpuProgram
    {
    public:
        GFXGpuProgram() {}
        GFXGpuProgram(const GFXGpuProgram&) = delete;
        virtual ~GFXGpuProgram() {}
    };
    GFX_DECL_SPTR(GFXGpuProgram);

}