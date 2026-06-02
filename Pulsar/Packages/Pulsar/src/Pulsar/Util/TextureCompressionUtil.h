#pragma once
#include "Pulsar/Assets/Texture2D.h"

namespace pulsar
{
    class TextureCompressionUtil
    {
    public:
        static constexpr int CACHE_VERSION = 1; // bump when algorithm changes

        static std::vector<uint8_t> Compress(
            std::vector<uint8_t> data,
            size_t width, size_t height, size_t channel,
            gfx::GFXTextureFormat format);
    };
} // namespace pulsar

