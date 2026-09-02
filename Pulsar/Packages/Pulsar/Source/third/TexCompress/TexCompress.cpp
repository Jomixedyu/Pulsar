#include <TexCompress/TexCompress.h>

#include <DirectXTex.h>
#include <astcenc.h>
#include <d3d11_1.h>
#include <wrl/client.h>

#include <algorithm>
#include <cstring>
#include <iterator>

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

    Microsoft::WRL::ComPtr<ID3D11Device> CreateD3D11Device()
    {
        static constexpr D3D_FEATURE_LEVEL featureLevels[] = {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
        };

        Microsoft::WRL::ComPtr<ID3D11Device> device;
        D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

        HRESULT hr = D3D11CreateDevice(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            0,
            featureLevels,
            static_cast<UINT>(std::size(featureLevels)),
            D3D11_SDK_VERSION,
            device.ReleaseAndGetAddressOf(),
            &featureLevel,
            nullptr);

        if (FAILED(hr) || featureLevel < D3D_FEATURE_LEVEL_11_0)
        {
            return nullptr;
        }

        return device;
    }

    bool TryCompressBC7OnGPU(
        const DirectX::Image& image,
        DXGI_FORMAT format,
        bool srgb,
        std::vector<std::uint8_t>& outData)
    {
        static thread_local Microsoft::WRL::ComPtr<ID3D11Device> device =
            CreateD3D11Device();

        if (!device)
        {
            return false;
        }

        DirectX::TEX_COMPRESS_FLAGS flags = DirectX::TEX_COMPRESS_BC7_QUICK;
        if (srgb)
        {
            flags |= DirectX::TEX_COMPRESS_SRGB;
        }

        DirectX::ScratchImage compressedImage;
        HRESULT hr = DirectX::Compress(
            device.Get(),
            image,
            format,
            flags,
            1.0f,
            compressedImage);

        if (FAILED(hr) || !compressedImage.GetPixels())
        {
            return false;
        }

        outData.resize(compressedImage.GetPixelsSize());
        std::memcpy(
            outData.data(),
            compressedImage.GetPixels(),
            compressedImage.GetPixelsSize());

        return true;
    }

    bool CompressBC7OnCPU(
        const DirectX::Image& image,
        DXGI_FORMAT format,
        bool srgb,
        std::vector<std::uint8_t>& outData)
    {
        ScopedOmpThreadLimit ompLimit;

        DirectX::TEX_COMPRESS_FLAGS flags = DirectX::TEX_COMPRESS_BC7_QUICK;
        if (srgb)
        {
            flags |= DirectX::TEX_COMPRESS_SRGB;
        }

        DirectX::ScratchImage compressedImage;
        HRESULT hr = DirectX::Compress(
            image,
            format,
            flags,
            DirectX::TEX_THRESHOLD_DEFAULT,
            compressedImage);

        if (FAILED(hr) || !compressedImage.GetPixels())
        {
            return false;
        }

        outData.resize(compressedImage.GetPixelsSize());
        std::memcpy(
            outData.data(),
            compressedImage.GetPixels(),
            compressedImage.GetPixelsSize());

        return true;
    }

    bool CompressASTCWithBlockSize(
        const std::uint8_t* source,
        std::size_t width,
        std::size_t height,
        std::size_t rowPitch,
        bool srgb,
        unsigned int blockWidth,
        unsigned int blockHeight,
        std::vector<std::uint8_t>& outData)
    {
        if (!source || width == 0 || height == 0 || rowPitch < width * 4)
        {
            return false;
        }

        constexpr unsigned int blockDepth = 1;
        constexpr std::size_t bytesPerBlock = 16;

        const std::size_t blocksX = (width + blockWidth - 1) / blockWidth;
        const std::size_t blocksY = (height + blockHeight - 1) / blockHeight;
        const std::size_t outputSize = blocksX * blocksY * bytesPerBlock;

        astcenc_config config{};
        astcenc_error status = astcenc_config_init(
            srgb ? ASTCENC_PRF_LDR_SRGB : ASTCENC_PRF_LDR,
            blockWidth,
            blockHeight,
            blockDepth,
            ASTCENC_PRE_FAST,
            0,
            &config);

        if (status != ASTCENC_SUCCESS)
        {
            return false;
        }

        astcenc_context* context = nullptr;
        status = astcenc_context_alloc(&config, 1, &context);
        if (status != ASTCENC_SUCCESS)
        {
            return false;
        }

        void* dataPtr = const_cast<std::uint8_t*>(source);
        astcenc_image image{};
        image.dim_x = static_cast<unsigned int>(width);
        image.dim_y = static_cast<unsigned int>(height);
        image.dim_z = 1;
        image.data_type = ASTCENC_TYPE_U8;
        image.data = &dataPtr;

        astcenc_swizzle swizzle{
            ASTCENC_SWZ_R,
            ASTCENC_SWZ_G,
            ASTCENC_SWZ_B,
            ASTCENC_SWZ_A,
        };

        outData.resize(outputSize);
        status = astcenc_compress_image(
            context,
            &image,
            &swizzle,
            outData.data(),
            outData.size(),
            0);

        astcenc_context_free(context);

        return status == ASTCENC_SUCCESS;
    }
}

namespace pulsar::texcompress
{
    bool CompressBC3(
        const std::uint8_t* source,
        std::size_t width,
        std::size_t height,
        std::size_t rowPitch,
        bool srgb,
        std::vector<std::uint8_t>& outData)
    {
        if (!source || width == 0 || height == 0 || rowPitch < width * 4)
        {
            return false;
        }

        DirectX::Image image{
            .width = width,
            .height = height,
            .format = srgb ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM,
            .rowPitch = rowPitch,
            .slicePitch = rowPitch * height,
            .pixels = const_cast<std::uint8_t*>(source),
        };

        DirectX::ScratchImage compressedImage;
        HRESULT hr = DirectX::Compress(
            image,
            srgb ? DXGI_FORMAT_BC3_UNORM_SRGB : DXGI_FORMAT_BC3_UNORM,
            srgb ? DirectX::TEX_COMPRESS_SRGB : DirectX::TEX_COMPRESS_DEFAULT,
            DirectX::TEX_THRESHOLD_DEFAULT,
            compressedImage);

        if (FAILED(hr) || !compressedImage.GetPixels())
        {
            return false;
        }

        outData.resize(compressedImage.GetPixelsSize());
        std::memcpy(
            outData.data(),
            compressedImage.GetPixels(),
            compressedImage.GetPixelsSize());

        return true;
    }

    bool CompressBC4(
        const std::uint8_t* source,
        std::size_t width,
        std::size_t height,
        std::size_t rowPitch,
        std::vector<std::uint8_t>& outData)
    {
        if (!source || width == 0 || height == 0 || rowPitch < width)
        {
            return false;
        }

        DirectX::Image image{
            .width = width,
            .height = height,
            .format = DXGI_FORMAT_R8_UNORM,
            .rowPitch = rowPitch,
            .slicePitch = rowPitch * height,
            .pixels = const_cast<std::uint8_t*>(source),
        };

        DirectX::ScratchImage compressedImage;
        HRESULT hr = DirectX::Compress(
            image,
            DXGI_FORMAT_BC4_UNORM,
            DirectX::TEX_COMPRESS_DEFAULT,
            DirectX::TEX_THRESHOLD_DEFAULT,
            compressedImage);

        if (FAILED(hr) || !compressedImage.GetPixels())
        {
            return false;
        }

        outData.resize(compressedImage.GetPixelsSize());
        std::memcpy(
            outData.data(),
            compressedImage.GetPixels(),
            compressedImage.GetPixelsSize());

        return true;
    }

    bool CompressBC5(
        const std::uint8_t* source,
        std::size_t width,
        std::size_t height,
        std::size_t rowPitch,
        std::vector<std::uint8_t>& outData)
    {
        if (!source || width == 0 || height == 0 || rowPitch < width * 4)
        {
            return false;
        }

        DirectX::Image image{
            .width = width,
            .height = height,
            .format = DXGI_FORMAT_R8G8B8A8_UNORM,
            .rowPitch = rowPitch,
            .slicePitch = rowPitch * height,
            .pixels = const_cast<std::uint8_t*>(source),
        };

        DirectX::ScratchImage compressedImage;
        HRESULT hr = DirectX::Compress(
            image,
            DXGI_FORMAT_BC5_UNORM,
            DirectX::TEX_COMPRESS_DEFAULT,
            DirectX::TEX_THRESHOLD_DEFAULT,
            compressedImage);

        if (FAILED(hr) || !compressedImage.GetPixels())
        {
            return false;
        }

        outData.resize(compressedImage.GetPixelsSize());
        std::memcpy(
            outData.data(),
            compressedImage.GetPixels(),
            compressedImage.GetPixelsSize());

        return true;
    }

    bool CompressBC6H(
        const float* source,
        std::size_t width,
        std::size_t height,
        std::size_t rowPitch,
        std::vector<std::uint8_t>& outData)
    {
        if (!source || width == 0 || height == 0 || rowPitch < width * 3 * sizeof(float))
        {
            return false;
        }

        DirectX::Image image{
            .width = width,
            .height = height,
            .format = DXGI_FORMAT_R32G32B32_FLOAT,
            .rowPitch = rowPitch,
            .slicePitch = rowPitch * height,
            .pixels = const_cast<std::uint8_t*>(
                reinterpret_cast<const std::uint8_t*>(source)),
        };

        DirectX::ScratchImage compressedImage;
        HRESULT hr = DirectX::Compress(
            image,
            DXGI_FORMAT_BC6H_SF16,
            DirectX::TEX_COMPRESS_DEFAULT,
            DirectX::TEX_THRESHOLD_DEFAULT,
            compressedImage);

        if (FAILED(hr) || !compressedImage.GetPixels())
        {
            return false;
        }

        outData.resize(compressedImage.GetPixelsSize());
        std::memcpy(
            outData.data(),
            compressedImage.GetPixels(),
            compressedImage.GetPixelsSize());

        return true;
    }

    bool CompressBC7(
        const std::uint8_t* source,
        std::size_t width,
        std::size_t height,
        std::size_t rowPitch,
        bool srgb,
        std::vector<std::uint8_t>& outData)
    {
        if (!source || width == 0 || height == 0 || rowPitch < width * 4)
        {
            return false;
        }

        DirectX::Image image{
            .width = width,
            .height = height,
            .format = srgb ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM,
            .rowPitch = rowPitch,
            .slicePitch = rowPitch * height,
            .pixels = const_cast<std::uint8_t*>(source),
        };

        DXGI_FORMAT format = srgb ? DXGI_FORMAT_BC7_UNORM_SRGB : DXGI_FORMAT_BC7_UNORM;

        if (TryCompressBC7OnGPU(image, format, srgb, outData))
        {
            return true;
        }

        return CompressBC7OnCPU(image, format, srgb, outData);
    }

    bool CompressASTC4x4(
        const std::uint8_t* source,
        std::size_t width,
        std::size_t height,
        std::size_t rowPitch,
        bool srgb,
        std::vector<std::uint8_t>& outData)
    {
        return CompressASTCWithBlockSize(
            source, width, height, rowPitch, srgb, 4, 4, outData);
    }

    bool CompressASTC6x6(
        const std::uint8_t* source,
        std::size_t width,
        std::size_t height,
        std::size_t rowPitch,
        bool srgb,
        std::vector<std::uint8_t>& outData)
    {
        return CompressASTCWithBlockSize(
            source, width, height, rowPitch, srgb, 6, 6, outData);
    }

    bool CompressASTC8x8(
        const std::uint8_t* source,
        std::size_t width,
        std::size_t height,
        std::size_t rowPitch,
        bool srgb,
        std::vector<std::uint8_t>& outData)
    {
        return CompressASTCWithBlockSize(
            source, width, height, rowPitch, srgb, 8, 8, outData);
    }
}
