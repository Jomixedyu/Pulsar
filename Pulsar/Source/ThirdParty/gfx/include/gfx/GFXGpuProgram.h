#pragma once
#include <vector>
#include <cstdint>

namespace gfx
{
    class GFXGpuProgram
    {
    public:
        GFXGpuProgram(const std::vector<uint8_t>& vert, const std::vector<uint8_t>& frag) {}
        GFXGpuProgram(const GFXGpuProgram&) = delete;
        virtual ~GFXGpuProgram() {}
    };
}