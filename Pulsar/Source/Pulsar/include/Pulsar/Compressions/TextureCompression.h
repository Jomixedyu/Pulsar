#pragma once
#include <vector>
#include <cstdint>

namespace pulsar
{
    class TextureCompression final
    {
    public:
        static std::vector<uint8_t> ASTC(uint8_t* data, size_t length, bool isSrgb, bool isHDR, int quality);
    };
}