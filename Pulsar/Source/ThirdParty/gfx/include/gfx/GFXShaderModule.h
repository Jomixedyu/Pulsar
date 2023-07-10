#pragma once
#include <vector>
#include <cstdint>

namespace gfx
{
    class GFXShaderModule
    {
    public:
        GFXShaderModule(const std::vector<uint8_t>& vert, const std::vector<uint8_t>& frag) {}
    };
}