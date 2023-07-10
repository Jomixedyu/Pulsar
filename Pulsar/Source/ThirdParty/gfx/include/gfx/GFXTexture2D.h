#pragma once
#include <cstdint>
#include "GFXTexture.h"

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

    class GFXTexture2D : public GFXTexture
    {
    protected:
        GFXTexture2D(int32_t width, int32_t height, int32_t channel, GFXSamplerConfig cfg, bool enableReadwrite)
            : m_width(width), m_height(height), m_channel(channel), m_samplerConfig(cfg), m_enableReadWrite(enableReadwrite)
        {

        }
    public:
        virtual ~GFXTexture2D() {}
    public:
        virtual int32_t GetWidth() const override { return m_width; }
        virtual int32_t GetHeight() const override { return m_height; }
        virtual int32_t GetChannelCount() const { return m_channel; }
        virtual const uint8_t* GetData() const = 0;
    protected:
        bool m_enableReadWrite;
        GFXSamplerConfig m_samplerConfig;
        int32_t m_width;
        int32_t m_height;
        int32_t m_channel;
    };
}