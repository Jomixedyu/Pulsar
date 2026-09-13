#include <gfx-vk/GFXVulkanResourceRegistry.h>

#include <gfx-vk/GFXVulkanBuffer.h>
#include <gfx-vk/GFXVulkanApplication.h>
#include <gfx-vk/GFXVulkanDescriptorSet.h>
#include <gfx-vk/GFXVulkanFrameBufferObject.h>
#include <gfx-vk/GFXVulkanGpuProgram.h>
#include <gfx-vk/GFXVulkanSampler.h>
#include <gfx-vk/GFXVulkanTexture.h>

namespace gfx
{
    GFXVulkanResourceRegistry::GFXVulkanResourceRegistry(GFXVulkanApplication* app)
        : m_app(app)
    {
    }

    GFXBufferPtr GFXVulkanResourceRegistry::CreateBuffer(const GFXBufferDesc& desc)
    {
        return RegisterResource(new GFXVulkanBuffer(this, desc));
    }

    GFXTexturePtr GFXVulkanResourceRegistry::CreateTexture2D(const GFXTextureCreateDesc& desc)
    {
        return RegisterResource(new GFXVulkanTexture(this, desc));
    }

    GFXTexturePtr GFXVulkanResourceRegistry::CreateTexture2DFromMemory(
        const uint8_t* imageData, size_t length,
        int width, int height,
        GFXTextureFormat format,
        const GFXSamplerConfig& samplerConfig)
    {
        GFXTextureCreateDesc desc{};
        desc.ImageData = imageData;
        desc.DataLength = length;
        desc.Width = width;
        desc.Height = height;
        desc.Depth = 1;
        desc.Format = format;
        desc.SamplerCfg = samplerConfig;
        desc.DataType = GFXTextureDataType::Texture2D;
        return CreateTexture2D(desc);
    }

    GFXTexturePtr GFXVulkanResourceRegistry::CreateTextureCube(int32_t size)
    {
        gfx::GFXTextureCreateDesc desc{};
        desc.Width = size;
        desc.Height = size;
        desc.Depth = 1;
        desc.ArrayLayers = 6;
        desc.Format = GFXTextureFormat::R16G16B16A16_SFloat;
        desc.TargetType = GFXTextureTargetType::ColorTarget;
        desc.DataType = GFXTextureDataType::TextureCube;

        return RegisterResource(new GFXVulkanTexture(this, desc));
    }

    GFXTexturePtr GFXVulkanResourceRegistry::CreateRenderTarget(
        int32_t width, int32_t height, GFXTextureTargetType type,
        GFXTextureFormat format, const GFXSamplerConfig& samplerCfg,
        uint32_t sampleCount, bool isTransientAttachment)
    {
        GFXTextureCreateDesc desc{};
        desc.Width = width;
        desc.Height = height;
        desc.Depth = 1;
        desc.Format = format;
        desc.TargetType = type;
        desc.DataType = GFXTextureDataType::Texture2D;
        desc.SampleCount = sampleCount;
        desc.IsTransientAttachment = isTransientAttachment;
        desc.SamplerCfg = samplerCfg;
        return RegisterResource(new GFXVulkanTexture(this, desc));
    }

    GFXFrameBufferObjectPtr GFXVulkanResourceRegistry::CreateFrameBufferObject(
        const array_list<GFXTexture2DViewPtr>& attachments)
    {
        return RegisterResource(new GFXVulkanFrameBufferObject(this, attachments));
    }

    GFXGpuProgramPtr GFXVulkanResourceRegistry::CreateGpuProgram(
        GFXGpuProgramStageFlags stage, const void* code, size_t length)
    {
        return RegisterResource(
            new GFXVulkanGpuProgram(this, stage, static_cast<const uint8_t*>(code), length));
    }

    GFXDescriptorSetLayoutPtr GFXVulkanResourceRegistry::CreateDescriptorSetLayout(
        const GFXDescriptorLayoutDesc* layouts, size_t layoutCount)
    {
        return RegisterResource(new GFXVulkanDescriptorSetLayout(this, layouts, layoutCount));
    }

    GFXSamplerPtr GFXVulkanResourceRegistry::CreateSampler(const GFXSamplerConfig& config)
    {
        return RegisterResource(new GFXVulkanSampler(this, config));
    }

    GFXApplication* GFXVulkanResourceRegistry::GetApplication() const
    {
        return m_app;
    }

    GFXVulkanApplication* GFXVulkanResourceRegistry::GetVulkanApplication() const
    {
        return m_app;
    }
}
