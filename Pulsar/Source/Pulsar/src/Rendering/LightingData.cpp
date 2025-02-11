#include "Rendering/LightingData.h"

#include "Application.h"

namespace pulsar
{

    LightManager::LightManager()
    {
        m_lightShaderParameters.reserve(m_bufferLength);
        m_pendingBuffer.reserve(m_bufferLength);

        auto bufferSize = sizeof(LightShaderParameter) * m_bufferLength;
        m_buffer = Application::GetGfxApp()->CreateBuffer(gfx::GFXBufferUsage::StructuredBuffer, bufferSize);

        array_list<gfx::GFXDescriptorSetLayoutInfo> layoutInfo;
        {
            gfx::GFXDescriptorSetLayoutInfo info {
                gfx::GFXDescriptorType::StructuredBuffer, gfx::GFXShaderStageFlags::VertexFragment, 0, 4
            };
            layoutInfo.push_back(info);
        }

        m_descriptorSetLayout = Application::GetGfxApp()->CreateDescriptorSetLayout(layoutInfo.data(), layoutInfo.size());
        m_descriptorSet = Application::GetGfxApp()->GetDescriptorManager()->GetDescriptorSet(m_descriptorSetLayout);

        m_descriptorSet->AddDescriptor("light", 0)->SetStructuredBuffer(m_buffer.get());
        m_descriptorSet->Submit();
    }
    LightManager::~LightManager()
    {
        m_buffer.reset();
        m_descriptorSetLayout.reset();
        m_descriptorSet.reset();
    }

    void LightManager::AddLight(LightShaderParameter* lightShaderParameter)
    {
        int id = -1;

        m_lightShaderParameters.push_back(lightShaderParameter);
        m_pendingBuffer.emplace_back();

        id = m_lightShaderParameters.size() - 1;

        m_ptr2index[lightShaderParameter] = id;

        MarkDirty(id);
    }

    void LightManager::RemoveLight(LightShaderParameter* lightShaderParameter)
    {
        int id = m_ptr2index[lightShaderParameter];

        m_lightShaderParameters[id] = m_lightShaderParameters.back();
        m_lightShaderParameters.pop_back();

        m_pendingBuffer[id] = m_pendingBuffer.back();
        m_pendingBuffer.pop_back();

        m_ptr2index.erase(lightShaderParameter);
        MarkDirty(id);
    }

    void LightManager::MarkDirty(int id)
    {
        m_dirtyList.push(id);
    }

    void LightManager::Update()
    {
        if (m_dirtyList.empty())
        {
            return;
        }
        if (m_bufferLength < GetLightCount())
        {
            m_bufferLength = size_t(m_bufferLength * 1.5);

            auto bufferSize = sizeof(LightShaderParameter) * m_bufferLength;
            m_buffer = Application::GetGfxApp()->CreateBuffer(gfx::GFXBufferUsage::StructuredBuffer, bufferSize);
            m_descriptorSet->FindByBinding(0)->SetStructuredBuffer(m_buffer.get());
            m_descriptorSet->Submit();
        }

        while (!m_dirtyList.empty())
        {
            auto id = m_dirtyList.front();
            m_dirtyList.pop();
            if (id >= m_pendingBuffer.size())
            {
                continue; // removed
            }

            m_pendingBuffer[id] = *m_lightShaderParameters[id];
        }
        m_buffer->Fill(m_pendingBuffer.data());
    }
    int LightManager::GetLightCount() const
    {
        return m_lightShaderParameters.size();
    }

}

