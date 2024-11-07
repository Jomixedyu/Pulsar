
#include "Util/TextureCompressionUtil.h"

#include "DirectXTex.h"
#include "stdfloat"

#ifdef _WIN32
    #include <DirectXTex/BC.h>
#endif

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
        std::vector<uint8_t> ret;
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
        case gfx::GFXTextureFormat::R8_UNorm:
            ret = _ResizeChannel<uint8_t>(std::move(data), width, height, channel, 1, 255);
            break;
        case gfx::GFXTextureFormat::R32G32B32A32_SFloat:

            ret = _ResizeChannel<float>(std::move(data), width, height, channel, 4, 1);
            break;
        default:
            assert(false);
        }
        return ret;
    }
} // namespace pulsar
