#pragma once
#include <cstdint>
#include <vector>
#include <memory>

namespace gfx
{
    extern std::vector<uint8_t> LoadImageFromMemory(
        const void* filedata, size_t size,
        int32_t* outWidth, int32_t* outHeight, int32_t* outChannel, int32_t reqComponents = 0, bool isSrgb = true);

    extern void LoadImageInfo(const void* filedata, size_t size, int32_t* outWidth, int32_t* outHeight, int32_t* outChannel);

    extern bool IsHDRImage(const void* filedata, size_t size);
}