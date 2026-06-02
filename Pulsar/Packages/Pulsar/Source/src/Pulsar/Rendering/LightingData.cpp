#include "Rendering/LightingData.h"

namespace pulsar
{

    LightManager::LightManager()
    {
        m_lightShaderParameters.reserve(m_bufferLength);
        m_pendingBuffer.reserve(m_bufferLength);
    }
    LightManager::~LightManager()
    {
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

        auto back = m_lightShaderParameters.back();
        m_lightShaderParameters[id] = m_lightShaderParameters.back();
        m_lightShaderParameters.pop_back();

        m_pendingBuffer[id] = m_pendingBuffer.back();
        m_pendingBuffer.pop_back();

        m_ptr2index[back] = id;

        m_ptr2index.erase(lightShaderParameter);
        MarkDirty(id);
    }

    void LightManager::MarkDirty(int id)
    {
        if (id < 0)
        {
            return;
        }
        m_dirtyList.push(id);
    }
    int LightManager::GetId(LightShaderParameter* light)
    {
        if (m_ptr2index.contains(light))
        {
            return m_ptr2index[light];
        }
        return -1;
    }

    void LightManager::Update()
    {
        while (!m_dirtyList.empty())
        {
            auto id = m_dirtyList.front();
            m_dirtyList.pop();
            if (id >= (int)m_pendingBuffer.size())
            {
                continue; // removed
            }
            m_pendingBuffer[id] = *m_lightShaderParameters[id];
        }
    }
    int LightManager::GetLightCount() const
    {
        return m_lightShaderParameters.size();
    }

}

