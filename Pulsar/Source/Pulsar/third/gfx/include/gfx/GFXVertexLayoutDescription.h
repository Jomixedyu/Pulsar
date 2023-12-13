#pragma once
#include <cstdint>
#include <vector>
#include "GFXInclude.h"

namespace gfx
{
    enum class GFXVertexInputDataFormat
    {
        R8G8B8A8_UInt,
        R32_SFloat,
        R32G32_SFloat,
        R32G32B32_SFloat,
        R32G32B32A32_SFloat,
    };

    struct GFXVertexInputAttribute
    {
        size_t Location;
        GFXVertexInputDataFormat Format;
        size_t Offset;
    };

    class GFXVertexLayoutDescription
    {
    public:
        uint32_t BindingPoint;
        uint32_t Stride;
        array_list<GFXVertexInputAttribute> Attributes;
    public:
        virtual ~GFXVertexLayoutDescription() {}
    };
    GFX_DECL_SPTR(GFXVertexLayoutDescription)
}