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

        // 创建 descriptor set layout: 3 个 ConstantBuffer binding (0, 1, 2)，全部在同一个 set
        gfx::GFXDescriptorSetLayoutDesc layoutDescs[3] = {
            {gfx::GFXDescriptorType::ConstantBuffer, gfx::GFXGpuProgramStageFlags::VertexFragment, 0, 1},
            {gfx::GFXDescriptorType::ConstantBuffer, gfx::GFXGpuProgramStageFlags::VertexFragment, 1, 1},
            {gfx::GFXDescriptorType::ConstantBuffer, gfx::GFXGpuProgramStageFlags::VertexFragment, 2, 1},
        };

        m_descriptorSetLayout = gfxApp->CreateDescriptorSetLayout(layoutDescs, 3);
        m_descriptorSet = gfxApp->GetDescriptorManager()->GetDescriptorSet(m_descriptorSetLayout);

        m_descriptorSet->AddDescriptor("CameraBuffer", 0)->SetConstantBuffer(m_cameraBuffer.get());
        m_descriptorSet->AddDescriptor("WorldBuffer", 1)->SetConstantBuffer(m_worldBuffer.get());
        m_descriptorSet->AddDescriptor("LightBuffer", 2)->SetConstantBuffer(m_lightsBuffer.get());
        m_descriptorSet->Submit();

        m_initialized = true;
    }

    void PerPassResources::Destroy()
    {
        if (!m_initialized)
            return;

        m_descriptorSet.reset();
        m_descriptorSetLayout.reset();
        m_cameraBuffer.reset();
        m_worldBuffer.reset();
        m_lightsBuffer.reset();
        m_initialized = false;
    }

    void PerPassResources::UpdateCamera(const PerPassCameraData& data)
    {
        if (m_cameraBuffer)
        {
            m_cameraBuffer->Fill(&data);
        }
    }

    void PerPassResources::UpdateWorld(const PerPassWorldData& data)
    {
        if (m_worldBuffer)
        {
            m_worldBuffer->Fill(&data);
        }
    }

    void PerPassResources::UpdateLights(const PerPassLightsBufferData& data)
    {
        if (m_lightsBuffer)
        {
            m_lightsBuffer->Fill(&data);
        }
    }

    void PerPassResources::Submit()
    {
        if (m_descriptorSet)
        {
            m_descriptorSet->Submit();
        }
    }
}
