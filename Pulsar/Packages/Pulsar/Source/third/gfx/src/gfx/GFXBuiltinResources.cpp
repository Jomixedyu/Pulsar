#include "gfx/GFXBuiltinResources.h"
#include "gfx/GFXApplication.h"

#include <vector>
#include <cstdint>

namespace gfx
{
    void GFXBuiltinResources::Initialize(GFXApplication* app)
    {
        m_app = app;

        // Shared zero buffer: device-local, usable as both UBO and SSBO, zero-filled once.
        GFXBufferDesc bufferDesc{};
        bufferDesc.Usage = OrFlag(GFXBufferUsage::ConstantBuffer, GFXBufferUsage::StructuredBuffer);
        bufferDesc.StorageType = GFXBufferMemoryPosition::DeviceLocal;
        bufferDesc.BufferSize = ZeroBufferSize;
        bufferDesc.ElementSize = 1;
        m_zeroBuffer = app->CreateBuffer(bufferDesc);

        std::vector<uint8_t> zeros(ZeroBufferSize, 0);
        m_zeroBuffer->Update(zeros.data());

        // 2x2 opaque-black RGBA8 texture.
        const uint8_t black[2 * 2 * 4] = {
            0, 0, 0, 255, 0, 0, 0, 255,
            0, 0, 0, 255, 0, 0, 0, 255,
        };
        GFXSamplerConfig sampler{};
        sampler.Filter = GFXSamplerFilter::Linear;
        sampler.AddressMode = GFXSamplerAddressMode::Repeat;

        m_black2D = app->CreateTexture2DFromMemory(
            black, sizeof(black), 2, 2, GFXTextureFormat::R8G8B8A8_UNorm, sampler);
        m_black2DView = m_black2D->Get2DView(0);
    }

    void GFXBuiltinResources::Terminate()
    {
        m_black2DView.reset();
        m_black2D.reset();
        m_zeroBuffer.reset();
        m_app = nullptr;
    }
}
