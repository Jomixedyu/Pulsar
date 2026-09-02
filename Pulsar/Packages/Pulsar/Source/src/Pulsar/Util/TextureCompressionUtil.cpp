#include "Util/TextureCompressionUtil.h"

#include <TexCompress/TexCompress.h>

#include <CoreLib/File.h>
#include <filesystem>
#include <cstring>

#ifdef _OPENMP
    #include <omp.h>
#endif

namespace
{
    uint64_t FNV1aHash(const uint8_t* data, size_t len)
    {
        uint64_t hash = 14695981039346656037ull;
        for (size_t i = 0; i < len; ++i)
        {
            hash ^= data[i];
            hash *= 1099511628211ull;
        }
        return hash;
    }

    uint64_t ComputeCacheFingerprint(
        const uint8_t* data,
        size_t dataLen,
        size_t width,
        size_t height,
        size_t channel,
        gfx::GFXTextureFormat format,
        int cacheVersion)
    {
        uint64_t hash = FNV1aHash(data, dataLen);
        auto mix = [&hash](uint64_t value)
        {
            hash ^= value;
            hash *= 1099511628211ull;
        };

        mix(static_cast<uint64_t>(width));
        mix(static_cast<uint64_t>(height));
        mix(static_cast<uint64_t>(channel));
        mix(static_cast<uint64_t>(format));
        mix(static_cast<uint64_t>(cacheVersion));
        return hash;
    }

    std::filesystem::path GetCacheDirectory()
    {
        auto cacheDir = std::filesystem::temp_directory_path() / "Pulsar" / "TextureCompressionCache";
        std::filesystem::create_directories(cacheDir);
        return cacheDir;
    }

    std::filesystem::path GetCachePath(uint64_t fingerprint)
    {
        return GetCacheDirectory() / (std::to_string(fingerprint) + ".cache");
    }

    bool TryLoadCache(uint64_t fingerprint, std::vector<uint8_t>& outData)
    {
        auto path = GetCachePath(fingerprint);
        if (!std::filesystem::exists(path))
        {
            return false;
        }

        auto bytes = jxcorlib::FileUtil::ReadAllBytes(path);
        if (bytes.size() < sizeof(uint64_t))
        {
            return false;
        }

        uint64_t fileFingerprint = 0;
        std::memcpy(&fileFingerprint, bytes.data(), sizeof(uint64_t));
        if (fileFingerprint != fingerprint)
        {
            return false;
        }

        outData.resize(bytes.size() - sizeof(uint64_t));
        if (!outData.empty())
        {
            std::memcpy(outData.data(), bytes.data() + sizeof(uint64_t), outData.size());
        }
        return true;
    }

    void SaveCache(uint64_t fingerprint, const std::vector<uint8_t>& data)
    {
        auto path = GetCachePath(fingerprint);
        std::vector<char> buffer;
        buffer.reserve(sizeof(uint64_t) + data.size());

        buffer.resize(sizeof(uint64_t));
        std::memcpy(buffer.data(), &fingerprint, sizeof(uint64_t));

        buffer.insert(
            buffer.end(),
            reinterpret_cast<const char*>(data.data()),
            reinterpret_cast<const char*>(data.data() + data.size()));

        jxcorlib::FileUtil::WriteAllBytes(
            jxcorlib::StrToU8Path(path.generic_string()),
            buffer.data(),
            buffer.size());
    }

    template <typename T>
    std::vector<uint8_t> ResizeChannel(
        std::vector<uint8_t> data,
        size_t width,
        size_t height,
        size_t channel,
        size_t newChannel,
        uint8_t fill)
    {
        if (channel == newChannel)
        {
            return data;
        }

        std::vector<uint8_t> result;
        result.resize(width * height * newChannel * sizeof(T));

        auto* resultData = reinterpret_cast<T*>(result.data());
        auto* sourceData = reinterpret_cast<T*>(data.data());
        const size_t pixelCount = width * height;

        #pragma omp parallel for
        for (int64_t pixel = 0; pixel < static_cast<int64_t>(pixelCount); ++pixel)
        {
            for (size_t component = 0; component < newChannel; ++component)
            {
                if (component < channel)
                {
                    resultData[pixel * newChannel + component] =
                        sourceData[pixel * channel + component];
                }
                else
                {
                    resultData[pixel * newChannel + component] = static_cast<T>(fill);
                }
            }
        }

        return result;
    }
}

namespace pulsar
{
    std::vector<uint8_t> TextureCompressionUtil::Compress(
        std::vector<uint8_t> data,
        size_t width,
        size_t height,
        size_t channel,
        gfx::GFXTextureFormat format)
    {
        uint64_t fingerprint = ComputeCacheFingerprint(
            data.data(),
            data.size(),
            width,
            height,
            channel,
            format,
            CACHE_VERSION);

        std::vector<uint8_t> result;
        if (TryLoadCache(fingerprint, result))
        {
            return result;
        }

        switch (format)
        {
        case gfx::GFXTextureFormat::BC3_SRGB:
        case gfx::GFXTextureFormat::BC3_UNorm:
            data = ResizeChannel<uint8_t>(
                std::move(data), width, height, channel, 4, 255);
            texcompress::CompressBC3(
                data.data(),
                width,
                height,
                width * 4,
                format == gfx::GFXTextureFormat::BC3_SRGB,
                result);
            break;

        case gfx::GFXTextureFormat::BC4_UNorm:
            data = ResizeChannel<uint8_t>(
                std::move(data), width, height, channel, 1, 0);
            texcompress::CompressBC4(
                data.data(),
                width,
                height,
                width,
                result);
            break;

        case gfx::GFXTextureFormat::BC5_UNorm:
            data = ResizeChannel<uint8_t>(
                std::move(data), width, height, channel, 4, 255);
            texcompress::CompressBC5(
                data.data(),
                width,
                height,
                width * 4,
                result);
            break;

        case gfx::GFXTextureFormat::BC6H_RGB_SFloat:
            texcompress::CompressBC6H(
                reinterpret_cast<const float*>(data.data()),
                width,
                height,
                width * 3 * sizeof(float),
                result);
            break;

        case gfx::GFXTextureFormat::BC7_SRGB:
        case gfx::GFXTextureFormat::BC7_UNorm:
            data = ResizeChannel<uint8_t>(
                std::move(data), width, height, channel, 4, 255);
            texcompress::CompressBC7(
                data.data(),
                width,
                height,
                width * 4,
                format == gfx::GFXTextureFormat::BC7_SRGB,
                result);
            break;

        case gfx::GFXTextureFormat::R8G8B8A8_UNorm:
        case gfx::GFXTextureFormat::R8G8B8A8_SRGB:
            result = ResizeChannel<uint8_t>(
                std::move(data), width, height, channel, 4, 255);
            break;

        case gfx::GFXTextureFormat::R16G16B16A16_SFloat:
            result = ResizeChannel<uint16_t>(
                std::move(data), width, height, channel, 4, 0x3C00);
            break;

        case gfx::GFXTextureFormat::R8_UNorm:
            result = ResizeChannel<uint8_t>(
                std::move(data), width, height, channel, 1, 255);
            break;

        case gfx::GFXTextureFormat::R16_UNorm:
            result = ResizeChannel<uint16_t>(
                std::move(data), width, height, channel, 1, 65535);
            break;

        case gfx::GFXTextureFormat::R32_SFloat:
            result = ResizeChannel<float>(
                std::move(data), width, height, channel, 1, 1);
            break;

        case gfx::GFXTextureFormat::R32G32B32A32_SFloat:
            result = ResizeChannel<float>(
                std::move(data), width, height, channel, 4, 1);
            break;

        default:
            assert(false);
            break;
        }

        SaveCache(fingerprint, result);
        return result;
    }
}
