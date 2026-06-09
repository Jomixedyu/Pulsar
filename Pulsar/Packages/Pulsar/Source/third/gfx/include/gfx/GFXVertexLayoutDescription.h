#pragma once
#include <cstdint>
#include <vector>
#include "GFXInclude.h"

namespace gfx
{
    enum class GFXVertexInputDataFormat : uint32_t
    {
        R8G8B8A8_UInt,
        R8G8B8A8_UNorm,
        R32_SFloat,
        R32G32_SFloat,
        R32G32B32_SFloat,
        R32G32B32A32_SFloat,
    };

    struct GFXVertexInputAttribute
    {
        uint32_t Location;
        uint32_t Offset;
        GFXVertexInputDataFormat Format;
    };

    // Pure CPU-side description of a vertex layout. Not a GPU resource:
    // it is consumed when building a graphics pipeline.
    struct GFXVertexLayoutDescription
    {
        uint32_t BindingPoint = 0;
        uint32_t Stride = 0;
        array_list<GFXVertexInputAttribute> Attributes;
    };
}
