#pragma once

#include <gfx/GFXResourceRegistry.h>

namespace gfx
{
    class GFXVulkanApplication;

    class GFXVulkanResourceRegistry final : public GFXResourceRegistry
    {
    public:
        explicit GFXVulkanResourceRegistry(GFXVulkanApplication* app);

        GFXBufferPtr CreateBuffer(const GFXBufferDesc& desc) override;
        GFXTexturePtr CreateTexture2D(const GFXTextureCreateDesc& desc) override;
        GFXTexturePtr CreateTexture2DFromMemory(
            const uint8_t* imageData, size_t length,
            int width, int height,
            GFXTextureFormat format,
            const GFXSamplerConfig& samplerConfig) override;
        GFXTexturePtr CreateTextureCube(int32_t size) override;
        GFXTexturePtr CreateRenderTarget(
            int32_t width, int32_t height, GFXTextureTargetType type,
            GFXTextureFormat format, const GFXSamplerConfig& samplerCfg,
            uint32_t sampleCount = 1, bool isTransientAttachment = false) override;
        GFXFrameBufferObjectPtr CreateFrameBufferObject(
            const array_list<GFXTexture2DViewPtr>& attachments) override;
        GFXGpuProgramPtr CreateGpuProgram(
            GFXGpuProgramStageFlags stage, const void* code, size_t length) override;
        GFXDescriptorSetLayoutPtr CreateDescriptorSetLayout(
            const GFXDescriptorLayoutDesc* layouts, size_t layoutCount) override;
        GFXSamplerPtr CreateSampler(const GFXSamplerConfig& config) override;

        GFXApplication* GetApplication() const override;
        GFXVulkanApplication* GetVulkanApplication() const;

    private:
        GFXVulkanApplication* m_app = nullptr;
    };
}
