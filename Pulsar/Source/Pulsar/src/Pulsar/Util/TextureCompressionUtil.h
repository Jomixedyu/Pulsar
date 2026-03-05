#pragma once
#include "Pulsar/Assets/Texture2D.h"

namespace pulsar
{
    class TextureCompressionUtil
    {
    public:
        static std::vector<uint8_t> Compress(
            std::vector<uint8_t> data,
            size_t width, size_t height, size_t channel,
            gfx::GFXTextureFormat format);
    };
} // namespace pulsar

