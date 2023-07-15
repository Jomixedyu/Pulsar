#pragma once
#include <cstdint>
#include <vector>
#include <memory>

namespace gfx
{
    extern std::vector<uint8_t> LoadImage(
        const uint8_t* filedata, size_t size,
        int32_t* width, int32_t* height, int32_t* channel, int32_t reqComponents);
}