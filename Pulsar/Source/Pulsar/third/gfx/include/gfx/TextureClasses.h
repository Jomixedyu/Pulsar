#pragma once
#include "TextureClasses.h"
#include <cstdint>

namespace gfx
{
    enum class GFXTextureFormat
    {
        R8_UNorm,
        R8G8B8A8_UNorm,
        R8G8B8A8_SRGB,
        R16G16B16A16_SFloat,
        R32G32B32A32_SFloat,
        B10G11R11_UFloat,
        BC3_SRGB,
        BC5_UNorm,
        BC6H_RGB_SFloat,
        D32_SFloat,
        D32_SFloat_S8_UInt,
        D24_UNorm_S8_UInt,
    };
    inline const char* to_string(GFXTextureFormat format)
    {
        switch (format)
        {
        case GFXTextureFormat::R8_UNorm:
            return "R8_UNorm";
        case GFXTextureFormat::R8G8B8A8_UNorm:
            return "R8G8B8A8_UNorm";
        case GFXTextureFormat::R8G8B8A8_SRGB:
            return "R8G8B8A8_SRGB";
        case GFXTextureFormat::R16G16B16A16_SFloat:
            return "R16G16B16A16_SFloat";
        case GFXTextureFormat::R32G32B32A32_SFloat:
            return "R32G32B32A32_SFloat";
        case GFXTextureFormat::B10G11R11_UFloat:
            return "B10G11R11_UFloat";
        case GFXTextureFormat::BC3_SRGB:
            return "BC3_SRGB";
        case GFXTextureFormat::BC5_UNorm:
            return "BC5_UNorm";
        case GFXTextureFormat::BC6H_RGB_SFloat:
            return "BC6H_RGB_SFloat";
        case GFXTextureFormat::D32_SFloat:
            return "D32_SFloat";
        case GFXTextureFormat::D32_SFloat_S8_UInt:
            return "D32_SFloat_S8_UInt";
        case GFXTextureFormat::D24_UNorm_S8_UInt:
            return "D24_UNorm_S8_UInt";
        }
        return nullptr;
    }

    enum class GFXTextureTargetType
    {
        None,
        ColorTarget = 0x01,
        DepthStencilTarget = 0x02,
        DepthTarget = 0x03,
    };

    enum class GFXTextureDataType
    {
        None,
        Texture2DArray,
        Texture2D,
        TextureCube,
        Texture3D
    };

    enum class GFXSamplerFilter
    {
        Nearest,
        Linear,
        Cubic
    };

    enum class GFXSamplerAddressMode
    {
        Repeat,
        MirroredRepeat,
        ClampToEdge,
    };

    struct GFXTextureSubresource
    {
        uint32_t BaseArrayLayer;
        uint32_t LayerCount;
    };

} // namespace gfx