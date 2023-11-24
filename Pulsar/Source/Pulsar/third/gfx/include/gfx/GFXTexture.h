#pragma once
#include <cstdint>

namespace gfx
{

    enum class GFXTextureFormat
    {
        R8,
        R8G8B8,
        R8G8B8A8,
        R8G8B8A8_SRGB,
        D32_SFloat,
        D32_SFloat_S8_UInt,
        D24_UNorm_S8_UInt
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
        //virtual void* GetTextureId() const = 0;
    };
}