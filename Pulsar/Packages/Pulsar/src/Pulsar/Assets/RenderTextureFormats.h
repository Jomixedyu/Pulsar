#pragma once
#include "Texture.h"

namespace pulsar
{
    CORELIB_DEF_ENUM(AssemblyObject_pulsar, pulsar, RenderTextureColorFormat,
        R8_UNorm,
        R16_UNorm,
        R32_SFloat,
        RGBA8_UNorm,
        RGBA8_SRGB,
        BGRA8_UNorm,
        RGBA16_SFloat,
        RGBA32_SFloat,
        R11G11B10_UFloat
    );

    CORELIB_DEF_ENUM(AssemblyObject_pulsar, pulsar, RenderTextureDepthFormat,
        None,
        D32_SFloat,
        D32_SFloat_S8_UInt,
        D24_UNorm_S8_UInt
    );
}

CORELIB_DECL_BOXING(pulsar::RenderTextureColorFormat, pulsar::BoxingRenderTextureColorFormat);
CORELIB_DECL_BOXING(pulsar::RenderTextureDepthFormat, pulsar::BoxingRenderTextureDepthFormat);

CORELIB_DECL_LIST(pulsar::RenderTextureColorFormat);
