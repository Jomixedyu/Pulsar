#include <gfx/GFXImage.h>

#define STBI_NO_STDIO
#define STBI_ONLY_JPEG
#define STBI_ONLY_PNG
#define STBI_ONLY_BMP
#define STBI_ONLY_PSD
#define STBI_ONLY_TGA
#define STBI_ONLY_HDR
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace gfx
{
    constexpr static float kToSrgb = 1.0f / 2.2f;
    constexpr static float kToLinear = 2.2f;

    std::vector<uint8_t> LoadImageFromMemory(
        const void* filedata, size_t size,
        int32_t* outWidth, int32_t* outHeight, int32_t* outChannel, int32_t reqComponents, bool isSrgb)
    {
        std::vector<uint8_t> ret;

        int width, height, channel;

        if (IsHDRImage(filedata, size))
        {
            auto pic = stbi_loadf_from_memory((stbi_uc*)filedata, (int)size, &width, &height, &channel, reqComponents);
            size_t bufSize = width * height * channel * sizeof(float);
            ret.resize(bufSize);
            memcpy(ret.data(), pic, bufSize);
            stbi_image_free(pic);
        }
        else
        {
            if (isSrgb)
            {
                auto pic = stbi_load_from_memory((stbi_uc*)filedata, (int)size, &width, &height, &channel, reqComponents);
                int rChannel = reqComponents == 0 ? channel : reqComponents;
                size_t bufSize = width * height * rChannel;
                ret.resize(bufSize);
                memcpy(ret.data(), pic, bufSize);
                stbi_image_free(pic);
                channel = rChannel;
            }
            else
            {
                // linear bitmap
                auto pic = stbi_loadf_from_memory((stbi_uc*)filedata, (int)size, &width, &height, &channel, reqComponents);

                int rChannel = reqComponents == 0 ? channel : reqComponents;
                size_t bufSize = width * height * rChannel;
                ret.resize(bufSize);
                size_t num = width * height;

    #pragma omp parallel for
                for (size_t i = 0; i < num; i++)
                {
                    for (size_t c = 0; c < rChannel; c++)
                    {
                        auto index = i * rChannel + c;
                        ret[index] = static_cast<uint8_t>(ceilf(pic[index] * 255));
                    }
                }

                stbi_image_free(pic);
            }
        }

        if (outWidth)
            *outWidth = width;
        if (outHeight)
            *outHeight = height;
        if (outChannel)
            *outChannel = channel;

        return ret;
    }

    void LoadImageInfo(const void* filedata, size_t size, int32_t* outWidth, int32_t* outHeight, int32_t* outChannel)
    {
        int width, height, channel;
        if (stbi_info_from_memory((const unsigned char*)filedata, (int)size, &width, &height, &channel))
        {
            if (outWidth)
                *outWidth = width;
            if (outHeight)
                *outHeight = height;
            if (outChannel)
                *outChannel = channel;
        }

    }

    bool IsHDRImage(const void* filedata, size_t size)
    {
        return stbi_is_hdr_from_memory((const unsigned char*)filedata, (int)size);
    }

} // namespace gfx
