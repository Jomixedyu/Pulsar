#include "Pulsar/Assets/Texture.h"

namespace pulsar
{
    hash_map<TextureCompressionFormat, gfx::GFXTextureFormat>* Texture::StaticGetFormatMapping(OSPlatform platform)
    {
        static hash_map<TextureCompressionFormat, gfx::GFXTextureFormat> map{
            {TextureCompressionFormat::ColorSRGBA_Compressed, gfx::GFXTextureFormat::BC3_SRGB},
            {TextureCompressionFormat::MaskRGBA_Compressed, gfx::GFXTextureFormat::BC3_UNorm},
            {TextureCompressionFormat::Alpha_Compressed, gfx::GFXTextureFormat::BC4_UNorm},
            {TextureCompressionFormat::NormalMap_Compressed, gfx::GFXTextureFormat::BC5_UNorm},
            {TextureCompressionFormat::HDR_Compressed, gfx::GFXTextureFormat::R32G32B32A32_SFloat},
            {TextureCompressionFormat::BitmapRGBA8, gfx::GFXTextureFormat::R8G8B8A8_UNorm},
            {TextureCompressionFormat::BitmapRGBA16, gfx::GFXTextureFormat::R16G16B16A16_SFloat},
            {TextureCompressionFormat::BitmapRGBA32, gfx::GFXTextureFormat::R32G32B32A32_SFloat},
            {TextureCompressionFormat::BitmapR8, gfx::GFXTextureFormat::R8_UNorm},
            {TextureCompressionFormat::BitmapR16, gfx::GFXTextureFormat::R16_UNorm},
            {TextureCompressionFormat::BitmapR32, gfx::GFXTextureFormat::R32_SFloat}};
        // {TextureCompressionFormat::HDR_Compressed, gfx::GFXTextureFormat::BC6H_RGB_SFloat}};
        return &map;
    }
} // namespace pulsar