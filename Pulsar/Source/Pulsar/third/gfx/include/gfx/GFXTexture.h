#pragma once
#include <cstdint>

namespace gfx
{

    enum class GFXTextureFormat
    {
        R8_UNorm,
        R8G8B8A8_UNorm,
        R8G8B8A8_SRGB,
        BC3_SRGB,
        BC5_UNorm,
        BC6H_RGB_SFloat,
        D32_SFloat,
        D32_SFloat_S8_UInt,
        D24_UNorm_S8_UInt
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

    struct GFXSamplerConfig
    {
        GFXSamplerFilter Filter;
        GFXSamplerAddressMode AddressMode;
    };

    class GFXTexture
    {
    public:
        virtual ~GFXTexture()
        {
        }

    public:
        virtual int32_t GetWidth() const = 0;
        virtual int32_t GetHeight() const = 0;
        virtual size_t GetTextureType() const = 0;
        // virtual void* GetTextureId() const = 0;
    };
} // namespace gfx