#pragma once

#include <string>
#include <string_view>
#include <cstdint>


namespace apatite
{
    class Bitmap;
    class Texture2D;

    namespace detail::ResourceInterface
    {
        unsigned char* LoadBitmap(std::string_view name, int* out_width, int* out_height, int* out_channel);
        void FreeBitmap(unsigned char* data);

    }

}
