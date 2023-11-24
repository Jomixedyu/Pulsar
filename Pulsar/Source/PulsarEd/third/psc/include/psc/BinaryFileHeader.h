#pragma once
#include <cstdint>
#include <vector>

namespace binf
{
    struct alignas(8) BinaryResourceInfo
    {
        struct hash128_t { uint64_t x, y; }; // 16 byte
        struct hash256_t { uint64_t x, y, z, w; }; // 32 byte
        union
        {
            struct
            {
                char ShortName[sizeof(hash128_t)];
                hash128_t Hash128;
            };
            char Name[sizeof(hash256_t)];
        };
        uint32_t Type;
        uint32_t Version;
        uint64_t Length;
        uint64_t Offset;
        uint32_t Flags;
        uint32_t Platform;
    };

    constexpr int kBinaryResourceInfoAlign = alignof(BinaryResourceInfo);
    constexpr int kBinaryResourceInfoSize = sizeof(BinaryResourceInfo);
    static_assert(kBinaryResourceInfoSize == 64);


    struct BinaryFileHeader
    {
        struct checkcode128_t
        {
            uint64_t x, y;
        };
        // 32 byte chunk
        uint64_t Magic;
        uint32_t Version;
        uint32_t Flags;
        checkcode128_t CheckCode;
        // 16 byte
        uint64_t ResourceCount;
        uint64_t ResourceTableOffset;
        // 16 byte
        uint64_t DataOffset;
        uint64_t PlaceA;
    };
    constexpr int kBinaryFileHeaderSize = sizeof(BinaryFileHeader);
    static_assert(kBinaryFileHeaderSize == 64);

}