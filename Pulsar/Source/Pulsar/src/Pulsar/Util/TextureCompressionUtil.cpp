
#include "Util/TextureCompressionUtil.h"

#include "DirectXTex.h"
#include "stdfloat"
#include <CoreLib/File.h>
#include <filesystem>
#include <cstring>

#ifdef _WIN32
    #include <DirectXTex/BC.h>
#endif

#ifdef _OPENMP
    #include <omp.h>
#endif

namespace
{
    class ScopedOmpThreadLimit
    {
    public:
        ScopedOmpThreadLimit()
        {
#ifdef _OPENMP
            m_oldThreads = omp_get_max_threads();
            int maxThreads = (std::max)(1, m_oldThreads - 2);
            omp_set_num_threads(maxThreads);
#endif
        }
        ~ScopedOmpThreadLimit()
        {
#ifdef _OPENMP
            omp_set_num_threads(m_oldThreads);
#endif
        }
    private:
        int m_oldThreads = 1;
    };
}

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
        const uint8_t* data, size_t dataLen,
        size_t width, size_t height, size_t channel,
        gfx::GFXTextureFormat format,
        int cacheVersion)
    {
        uint64_t hash = FNV1aHash(data, dataLen);
        auto mix = [&hash](uint64_t v)
        {
            hash ^= v;
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
            return false;

        auto bytes = jxcorlib::FileUtil::ReadAllBytes(path);
        if (bytes.size() < sizeof(uint64_t))
            return false;

        uint64_t fileFingerprint;
        std::memcpy(&fileFingerprint, bytes.data(), sizeof(uint64_t));
        if (fileFingerprint != fingerprint)
            return false;

        outData.resize(bytes.size() - sizeof(uint64_t));
        if (!outData.empty())
            std::memcpy(outData.data(), bytes.data() + sizeof(uint64_t), outData.size());
        return true;
    }

    void SaveCache(uint64_t fingerprint, const std::vector<uint8_t>& data)
    {
        auto path = GetCachePath(fingerprint);
        std::vector<char> buffer;
        buffer.reserve(sizeof(uint64_t) + data.size());

        buffer.resize(sizeof(uint64_t));
        std::memcpy(buffer.data(), &fingerprint, sizeof(uint64_t));

        buffer.insert(buffer.end(),
            reinterpret_cast<const char*>(data.data()),
            reinterpret_cast<const char*>(data.data() + data.size()));

        jxcorlib::FileUtil::WriteAllBytes(
            jxcorlib::StrToU8Path(path.generic_string()),
            buffer.data(), buffer.size());
    }
}

namespace pulsar
{
    template <typename _TyData>
    static std::vector<uint8_t> _ResizeChannel(std::vector<uint8_t> data, size_t width, size_t height, size_t channel, size_t newChannel, uint8_t fill)
    {
        if (channel == newChannel)
        {
            return std::move(data);
        }

        std::vector<uint8_t> ret;

        ret.resize(width * height * newChannel * sizeof(_TyData));

        auto retp = (_TyData*)ret.data();
        auto datap = (_TyData*)data.data();

        const bool greater = newChannel > channel;

        const size_t pixelCount = width * height;

        #pragma omp parallel for
        for (int p = 0; p < pixelCount; ++p)
        {
            for (size_t c = 0; c < newChannel; ++c)
            {
                if (greater)
                {
                    if (c >= channel)
                    {
                        //fill
                        retp[p * newChannel + c] = fill;
                    }
                    else
                    {
                        retp[p * newChannel + c] = datap[p * channel + c];
                    }
                }
                else
                {
                    if (c >= newChannel)
                    {
                        //trim
                    }
                    else
                    {
                        retp[p * newChannel + c] = datap[p * channel + c];
                    }
                }

            }
        }

        return ret;
    }

    std::vector<uint8_t> TextureCompressionUtil::Compress(
        std::vector<uint8_t> data,
        size_t width, size_t height, size_t channel,
        gfx::GFXTextureFormat format)
    {
        uint64_t fingerprint = ComputeCacheFingerprint(
            data.data(), data.size(),
            width, height, channel,
            format, CACHE_VERSION);

        std::vector<uint8_t> ret;
        if (TryLoadCache(fingerprint, ret))
            return ret;

        switch (format)
        {
#ifdef _WIN32
        case gfx::GFXTextureFormat::BC3_SRGB: {

            if (channel != 4)
            {
                data = _ResizeChannel<uint8_t>(std::move(data), width, height, 3, 4, 255);
            }

            DirectX::Image img{
                .width = width,
                .height = height,
                .format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
                .rowPitch = width * 4,
                .slicePitch = width * height * 4,
                .pixels = data.data()
            };
            DirectX::ScratchImage SImg;
            DirectX::Compress(
                img,
                DXGI_FORMAT_BC3_UNORM_SRGB,
                DirectX::TEX_COMPRESS_SRGB,
                DirectX::TEX_THRESHOLD_DEFAULT,
                SImg);
            ret.resize(SImg.GetPixelsSize());
            std::memcpy(ret.data(), SImg.GetPixels(), SImg.GetPixelsSize());
            break;
        }
        case gfx::GFXTextureFormat::BC3_UNorm: {

            if (channel != 4)
            {
                data = _ResizeChannel<uint8_t>(std::move(data), width, height, 3, 4, 255);
            }

            DirectX::Image img{
                .width = width,
                .height = height,
                .format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM,
                .rowPitch = width * 4,
                .slicePitch = width * height * 4,
                .pixels = data.data()
            };
            DirectX::ScratchImage SImg;
            DirectX::Compress(
                img,
                DXGI_FORMAT_BC3_UNORM,
                DirectX::TEX_COMPRESS_DEFAULT,
                DirectX::TEX_THRESHOLD_DEFAULT,
                SImg);
            ret.resize(SImg.GetPixelsSize());
            std::memcpy(ret.data(), SImg.GetPixels(), SImg.GetPixelsSize());
            break;
        }
        case gfx::GFXTextureFormat::BC7_SRGB: {

            if (channel != 4)
            {
                data = _ResizeChannel<uint8_t>(std::move(data), width, height, 3, 4, 255);
            }

            DirectX::Image img{
                .width = width,
                .height = height,
                .format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
                .rowPitch = width * 4,
                .slicePitch = width * height * 4,
                .pixels = data.data()
            };
            DirectX::ScratchImage SImg;
            ScopedOmpThreadLimit ompLimit;
            DirectX::Compress(
                img,
                DXGI_FORMAT_BC7_UNORM_SRGB,
                DirectX::TEX_COMPRESS_SRGB | DirectX::TEX_COMPRESS_PARALLEL | DirectX::TEX_COMPRESS_BC7_QUICK,
                DirectX::TEX_THRESHOLD_DEFAULT,
                SImg);
            ret.resize(SImg.GetPixelsSize());
            std::memcpy(ret.data(), SImg.GetPixels(), SImg.GetPixelsSize());
            break;
        }
        case gfx::GFXTextureFormat::BC7_UNorm: {

            if (channel != 4)
            {
                data = _ResizeChannel<uint8_t>(std::move(data), width, height, 3, 4, 255);
            }

            DirectX::Image img{
                .width = width,
                .height = height,
                .format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM,
                .rowPitch = width * 4,
                .slicePitch = width * height * 4,
                .pixels = data.data()
            };
            DirectX::ScratchImage SImg;
            ScopedOmpThreadLimit ompLimit;
            DirectX::Compress(
                img,
                DXGI_FORMAT_BC7_UNORM,
                DirectX::TEX_COMPRESS_DEFAULT | DirectX::TEX_COMPRESS_PARALLEL | DirectX::TEX_COMPRESS_BC7_QUICK,
                DirectX::TEX_THRESHOLD_DEFAULT,
                SImg);
            ret.resize(SImg.GetPixelsSize());
            std::memcpy(ret.data(), SImg.GetPixels(), SImg.GetPixelsSize());
            break;
        }
        case gfx::GFXTextureFormat::BC4_UNorm: {
            data = _ResizeChannel<uint8_t>(std::move(data), width, height, channel, 1, 0);
            DirectX::Image img{
                .width = width,
                .height = height,
                .format = DXGI_FORMAT::DXGI_FORMAT_R8_UNORM,
                .rowPitch = width,
                .slicePitch = width * height,
                .pixels = data.data()
            };
            DirectX::ScratchImage SImg;
            DirectX::Compress(
                img, DXGI_FORMAT_BC4_UNORM,
                DirectX::TEX_COMPRESS_DEFAULT,
                DirectX::TEX_THRESHOLD_DEFAULT,
                SImg);
            ret.resize(SImg.GetPixelsSize());
            std::memcpy(ret.data(), SImg.GetPixels(), SImg.GetPixelsSize());
            break;
        }
        case gfx::GFXTextureFormat::BC5_UNorm: {
            data = _ResizeChannel<uint8_t>(std::move(data), width, height, channel, 4, 255);
            DirectX::Image img{
                .width = width,
                .height = height,
                .format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM,
                .rowPitch = width * 4,
                .slicePitch = width * height * 4,
                .pixels = data.data()
            };
            DirectX::ScratchImage SImg;
            DirectX::Compress(
                img, DXGI_FORMAT_BC5_UNORM,
                DirectX::TEX_COMPRESS_DEFAULT,
                DirectX::TEX_THRESHOLD_DEFAULT,
                SImg);
            ret.resize(SImg.GetPixelsSize());
            std::memcpy(ret.data(), SImg.GetPixels(), SImg.GetPixelsSize());
            break;
        }
        case gfx::GFXTextureFormat::BC6H_RGB_SFloat: {
            DirectX::Image img{
                .width = width,
                .height = height,
                .format = DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT,
                .rowPitch = width * 3 * sizeof(float),
                .slicePitch = width * height * 3 * sizeof(float),
                .pixels = data.data()
            };
            DirectX::ScratchImage SImg;
            DirectX::Compress(
                img,
                DXGI_FORMAT_BC6H_SF16,
                DirectX::TEX_COMPRESS_DEFAULT | DirectX::TEX_COMPRESS_PARALLEL,
                DirectX::TEX_THRESHOLD_DEFAULT,
                SImg);
            ret.resize(SImg.GetPixelsSize());
            std::memcpy(ret.data(), SImg.GetPixels(), SImg.GetPixelsSize());
            break;
        }
#endif
        case gfx::GFXTextureFormat::R8G8B8A8_UNorm:
        case gfx::GFXTextureFormat::R8G8B8A8_SRGB:
            ret = _ResizeChannel<uint8_t>(std::move(data), width, height, channel, 4, 255);
            break;
        case gfx::GFXTextureFormat::R16G16B16A16_SFloat:
            ret = _ResizeChannel<uint16_t>(std::move(data), width, height, channel, 4, 0x3C00);
            break;
        case gfx::GFXTextureFormat::R8_UNorm:
            ret = _ResizeChannel<uint8_t>(std::move(data), width, height, channel, 1, 255);
            break;
        case gfx::GFXTextureFormat::R16_UNorm:
            ret = _ResizeChannel<uint16_t>(std::move(data), width, height, channel, 1, 65535);
            break;
        case gfx::GFXTextureFormat::R32_SFloat:
            ret = _ResizeChannel<float>(std::move(data), width, height, channel, 1, 1);
            break;
        case gfx::GFXTextureFormat::R32G32B32A32_SFloat:
            ret = _ResizeChannel<float>(std::move(data), width, height, channel, 4, 1);
            break;
        default:
            assert(false);
        }
        SaveCache(fingerprint, ret);
        return ret;
    }
} // namespace pulsar
