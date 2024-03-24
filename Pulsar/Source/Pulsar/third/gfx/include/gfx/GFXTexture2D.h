#pragma once
#include "GFXInclude.h"
#include "GFXTexture.h"

namespace gfx
{
    class GFXTexture2D : public GFXTexture
    {
    protected:
        GFXTexture2D(int32_t width, int32_t height, GFXSamplerConfig cfg, bool enableReadwrite)
            : m_width(width), m_height(height), m_samplerConfig(cfg), m_enableReadWrite(enableReadwrite)
        {

        }
    public:
        ~GFXTexture2D() override
        {
        }
    public:
        virtual int32_t GetWidth() const override { return m_width; }
        virtual int32_t GetHeight() const override { return m_height; }
        virtual const uint8_t* GetData() const = 0;
    protected:
        bool m_enableReadWrite;
        GFXSamplerConfig m_samplerConfig;
        int32_t m_width;
        int32_t m_height;
    };
    GFX_DECL_SPTR(GFXTexture2D);
}