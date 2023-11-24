#pragma once
#include <cstdint>

namespace gfx
{
    enum class SamplerFilter : uint8_t
    {
        Nearest,
        Linear,
        Cubic
    };
    enum class SamplerAddressMode : uint8_t
    {
        Repeat,
        Mirror,
        Clamp,
    };
}