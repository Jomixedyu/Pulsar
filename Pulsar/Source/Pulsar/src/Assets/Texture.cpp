#include "Pulsar/Assets/Texture.h"

namespace pulsar
{
    hash_map<TextureCompressionFormat, gfx::GFXTextureFormat>* Texture::StaticGetFormatMapping(OSPlatform platform)
    {
        static hash_map<TextureCompressionFormat, gfx::GFXTextureFormat> map{
            {TextureCompressionFormat::ColorSRGB_Compressed, gfx::GFXTextureFormat::BC3_SRGB},
            {TextureCompressionFormat::BitmapRGBA, gfx::GFXTextureFormat::R8G8B8A8_UNorm},
            {TextureCompressionFormat::Gray, gfx::GFXTextureFormat::R8_UNorm},
            {TextureCompressionFormat::NormalMap_Compressed, gfx::GFXTextureFormat::BC5_UNorm},
            {TextureCompressionFormat::HDR_Compressed, gfx::GFXTextureFormat::R32G32B32A32_SFloat}};
        // {TextureCompressionFormat::HDR_Compressed, gfx::GFXTextureFormat::BC6H_RGB_SFloat}};
        return &map;
    }
} // namespace pulsar