#pragma once
#include <cstdint>

namespace gfx
{
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