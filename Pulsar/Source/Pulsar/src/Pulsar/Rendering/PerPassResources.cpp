#include "Rendering/PerPassResources.h"

#include <Pulsar/Application.h>

namespace pulsar
{
    void PerPassResources::Initialize()
    {
        if (m_initialized)
            return;

        auto gfxApp = Application::GetGfxApp();

        // 创建 3 个 cbuffer
        {
            gfx::GFXBufferDesc desc{};
            desc.Usage = gfx::GFXBufferUsage::ConstantBuffer;
            desc.StorageType = gfx::GFXBufferMemoryPosition::VisibleOnDevice;

            desc.BufferSize = sizeof(PerPassCameraData);
            m_cameraBuffer = gfxApp->CreateBuffer(desc);

            desc.BufferSize = sizeof(PerPassWorldData);
            m_worldBuffer = gfxApp->CreateBuffer(desc);

            desc.BufferSize = sizeof(PerPassLightsBufferData);
            m_lightsBuffer = gfxApp->CreateBuffer(desc);
        }

        m_initialized = true;
    }

    void PerPassResources::Destroy()
    {
        if (!m_initialized)
            return;

        m_layoutCache.clear();
        m_cameraBuffer.reset();
        m_worldBuffer.reset();
        m_lightsBuffer.reset();
        m_initialized = false;
    }

    void PerPassResources::UpdateCamera(const PerPassCameraData& data)
    {
        if (m_cameraBuffer)
            m_cameraBuffer->Fill(&data);
    }

    void PerPassResources::UpdateWorld(const PerPassWorldData& data)
    {
        if (m_worldBuffer)
            m_worldBuffer->Fill(&data);
    }

    void PerPassResources::UpdateLights(const PerPassLightsBufferData& data)
    {
        if (m_lightsBuffer)
            m_lightsBuffer->Fill(&data);
    }

    gfx::GFXDescriptorSetLayout_sp PerPassResources::GetLayout(const std::string& passName)
    {
        auto it = m_layoutCache.find(passName);
        if (it != m_layoutCache.end())
            return it->second;

        auto gfxApp = Application::GetGfxApp();
        std::vector<gfx::GFXDescriptorSetLayoutDesc> descs;

        if (passName == "Forward")
        {
            descs = {
                {gfx::GFXDescriptorType::ConstantBuffer,       gfx::GFXGpuProgramStageFlags::VertexFragment, 0, 1},
                {gfx::GFXDescriptorType::ConstantBuffer,       gfx::GFXGpuProgramStageFlags::VertexFragment, 1, 1},
                {gfx::GFXDescriptorType::ConstantBuffer,       gfx::GFXGpuProgramStageFlags::VertexFragment, 2, 1},
                {gfx::GFXDescriptorType::CombinedImageSampler, gfx::GFXGpuProgramStageFlags::Fragment,       3, 1},
                {gfx::GFXDescriptorType::CombinedImageSampler, gfx::GFXGpuProgramStageFlags::Fragment,       4, 1},
            };
        }
        else if (passName == "PostProcess")
        {
            descs = {
                {gfx::GFXDescriptorType::ConstantBuffer,       gfx::GFXGpuProgramStageFlags::VertexFragment, 0, 1},
                {gfx::GFXDescriptorType::ConstantBuffer,       gfx::GFXGpuProgramStageFlags::VertexFragment, 1, 1},
                {gfx::GFXDescriptorType::CombinedImageSampler, gfx::GFXGpuProgramStageFlags::Fragment,       3, 1},
            };
        }
        else if (passName == "ShadowCaster")
        {
            descs = {
                {gfx::GFXDescriptorType::ConstantBuffer, gfx::GFXGpuProgramStageFlags::VertexFragment, 0, 1},
                {gfx::GFXDescriptorType::ConstantBuffer, gfx::GFXGpuProgramStageFlags::VertexFragment, 1, 1},
            };
        }
        else
        {
            // 默认：标准 3 CBV
            descs = {
                {gfx::GFXDescriptorType::ConstantBuffer, gfx::GFXGpuProgramStageFlags::VertexFragment, 0, 1},
                {gfx::GFXDescriptorType::ConstantBuffer, gfx::GFXGpuProgramStageFlags::VertexFragment, 1, 1},
                {gfx::GFXDescriptorType::ConstantBuffer, gfx::GFXGpuProgramStageFlags::VertexFragment, 2, 1},
            };
        }

        auto layout = gfxApp->CreateDescriptorSetLayout(descs.data(), static_cast<uint32_t>(descs.size()));
        m_layoutCache[passName] = layout;
        return layout;
    }

    gfx::GFXDescriptorSet_sp PerPassResources::AllocateSet(gfx::GFXDescriptorSetLayout_sp layout) const
    {
        auto gfxApp = Application::GetGfxApp();
        return gfxApp->GetDescriptorManager()->GetDescriptorSet(layout);
    }

    void PerPassResources::WriteCameraToSet(gfx::GFXDescriptorSet* set) const
    {
        if (!set) return;
        auto* camDesc = set->FindByBinding(0);
        if (!camDesc) camDesc = set->AddDescriptor("CameraBuffer", 0);
        if (camDesc) camDesc->SetConstantBuffer(m_cameraBuffer.get());
    }

    void PerPassResources::WriteWorldToSet(gfx::GFXDescriptorSet* set) const
    {
        if (!set) return;
        auto* worldDesc = set->FindByBinding(1);
        if (!worldDesc) worldDesc = set->AddDescriptor("WorldBuffer", 1);
        if (worldDesc) worldDesc->SetConstantBuffer(m_worldBuffer.get());
    }

    void PerPassResources::WriteLightsToSet(gfx::GFXDescriptorSet* set) const
    {
        if (!set) return;
        auto* lightDesc = set->FindByBinding(2);
        if (!lightDesc) lightDesc = set->AddDescriptor("LightBuffer", 2);
        if (lightDesc) lightDesc->SetConstantBuffer(m_lightsBuffer.get());
    }

    void PerPassResources::WriteStandardBuffers(gfx::GFXDescriptorSet* set) const
    {
        WriteCameraToSet(set);
        WriteWorldToSet(set);
        WriteLightsToSet(set);
    }

    void PerPassResources::WriteTexture(gfx::GFXDescriptorSet* set, uint32_t binding, gfx::GFXTexture2DView* view) const
    {
        if (!set) return;
        auto* desc = set->FindByBinding(binding);
        if (!desc) desc = set->AddDescriptor("Texture", binding);
        if (view && desc)
            desc->SetTextureSampler2D(view);
    }

    void PerPassResources::Submit(gfx::GFXDescriptorSet* set) const
    {
        if (set)
            set->Submit();
    }
}
