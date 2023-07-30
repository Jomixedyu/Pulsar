#pragma once
#include <cstdint>
#include <vector>

namespace gfx
{
    enum class GFXVertexInputDataFormat
    {
        R32G32_SFloat,
        R32G32B32_SFloat,
        R32G32B32A32_SFloat,
    };

    struct GFXVertexInputAttribute
    {
        GFXVertexInputDataFormat Format;
        size_t Offset;
    };

    class GFXVertexLayoutDescription
    {
    public:
        uint32_t BindingPoint;
        uint32_t Stride;
        std::vector<GFXVertexInputAttribute> Attributes;
    public:
        virtual ~GFXVertexLayoutDescription() {}
    };
}