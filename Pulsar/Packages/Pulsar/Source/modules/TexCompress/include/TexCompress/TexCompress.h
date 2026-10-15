#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace pulsar::texcompress
{
    bool CompressBC3(
        const std::uint8_t* source,
        std::size_t width,
        std::size_t height,
        std::size_t rowPitch,
        bool srgb,
        std::vector<std::uint8_t>& outData);

    bool CompressBC4(
        const std::uint8_t* source,
        std::size_t width,
        std::size_t height,
        std::size_t rowPitch,
        std::vector<std::uint8_t>& outData);

    bool CompressBC5(
        const std::uint8_t* source,
        std::size_t width,
        std::size_t height,
        std::size_t rowPitch,
        std::vector<std::uint8_t>& outData);

    bool CompressBC6H(
        const float* source,
        std::size_t width,
        std::size_t height,
        std::size_t rowPitch,
        std::vector<std::uint8_t>& outData);

    bool CompressBC7(
        const std::uint8_t* source,
        std::size_t width,
        std::size_t height,
        std::size_t rowPitch,
        bool srgb,
        std::vector<std::uint8_t>& outData);

    bool CompressASTC4x4(
        const std::uint8_t* source,
        std::size_t width,
        std::size_t height,
        std::size_t rowPitch,
        bool srgb,
        std::vector<std::uint8_t>& outData);

    bool CompressASTC6x6(
        const std::uint8_t* source,
        std::size_t width,
        std::size_t height,
        std::size_t rowPitch,
        bool srgb,
        std::vector<std::uint8_t>& outData);

    bool CompressASTC8x8(
        const std::uint8_t* source,
        std::size_t width,
        std::size_t height,
        std::size_t rowPitch,
        bool srgb,
        std::vector<std::uint8_t>& outData);
}
