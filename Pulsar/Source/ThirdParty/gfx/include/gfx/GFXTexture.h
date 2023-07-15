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

    };
}