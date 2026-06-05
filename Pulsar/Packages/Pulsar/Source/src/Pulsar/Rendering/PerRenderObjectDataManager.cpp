#include "PerRenderObjectDataManager.h"
#include <Pulsar/Application.h>

namespace pulsar
{
    gfx::GFXBuffer* PerRenderObjectDataManager::GetBuffer() const
    {
        if (!m_buffer.IsValid()) return nullptr;
        return Application::GetGfxApp()->GetResourceManager()->GetBuffer(m_buffer);
    }

    void PerRenderObjectDataManager::Initialize()
    {
        if (m_buffer.IsValid()) return; // already initialized

        Grow(kInitialCapacity);

        // Create dummy extra set (set2 for renderers without extra data)
        gfx::GFXDescriptorSetLayoutDesc dummyDesc{};
        m_dummyExtraLayout = Application::GetGfxApp()->CreateDescriptorSetLayout(&dummyDesc, 0);
        m_dummyExtraSet = Application::GetGfxApp()->GetDescriptorManager()->GetDescriptorSet(m_dummyExtraLayout);
        m_dummyExtraSet->Submit();
    }

    void PerRenderObjectDataManager::Destroy()
    {
        if (!m_buffer.IsValid()) return; // already destroyed
        m_dummyExtraSet.reset();
        m_dummyExtraLayout.reset();
        auto& cmdList = Application::GetGfxApp()->GetImmediateCommandList();
        cmdList.Destroy(m_buffer);
        m_buffer = gfx::BufferHandle{};
        m_cpuData.clear();
        m_slotUsed.clear();
        m_freeSlots.clear();
        m_nextSlot = 0;
        m_capacity = 0;
    }

    void PerRenderObjectDataManager::Grow(uint32_t newCapacity)
    {
        if (newCapacity <= m_capacity) return;

        auto& cmdList = Application::GetGfxApp()->GetImmediateCommandList();

        gfx::GFXBufferDesc desc{};
        desc.Usage = gfx::GFXBufferUsage::ConstantBuffer;
        desc.StorageType = gfx::GFXBufferMemoryPosition::VisibleOnHost;
        desc.BufferSize = newCapacity * sizeof(PerRenderObjectData);
        desc.ElementSize = sizeof(PerRenderObjectData);
        auto newBuffer = cmdList.CreateBuffer(desc);

        // Copy existing data if any
        if (m_buffer.IsValid() && m_capacity > 0)
        {
            if (auto* oldBuffer = Application::GetGfxApp()->GetResourceManager()->GetBuffer(m_buffer))
            {
                // TODO: copy old data to new buffer
            }
            cmdList.Destroy(m_buffer);
        }

        m_buffer = newBuffer;
        m_cpuData.resize(newCapacity);
        m_slotUsed.resize(newCapacity, false);
        m_capacity = newCapacity;
    }

    uint32_t PerRenderObjectDataManager::AllocSlot()
    {
        if (!m_buffer.IsValid()) Initialize();

        if (!m_freeSlots.empty())
        {
            uint32_t slot = m_freeSlots.back();
            m_freeSlots.pop_back();
            m_slotUsed[slot] = true;
            return slot;
        }

        if (m_nextSlot >= m_capacity)
        {
            uint32_t newCapacity = m_capacity == 0 ? kInitialCapacity : m_capacity * 2;
            Grow(newCapacity);
        }

        uint32_t slot = m_nextSlot++;
        m_slotUsed[slot] = true;
        return slot;
    }

    void PerRenderObjectDataManager::FreeSlot(uint32_t slot)
    {
        if (slot >= m_capacity || !m_slotUsed[slot]) return;
        m_slotUsed[slot] = false;
        m_freeSlots.push_back(slot);
    }

    void PerRenderObjectDataManager::SetData(uint32_t slot, const PerRenderObjectData& data)
    {
        if (slot >= m_capacity) return;
        m_cpuData[slot] = data;
    }

    void PerRenderObjectDataManager::BeginFrame()
    {
        // Prepare for frame (optional: clear dirty tracking)
    }

    void PerRenderObjectDataManager::EndFrame()
    {
        if (!m_buffer.IsValid() || m_nextSlot == 0) return;

        // Upload all active slots in one Fill
        auto* buffer = Application::GetGfxApp()->GetResourceManager()->GetBuffer(m_buffer);
        if (buffer)
        {
            buffer->Fill(m_cpuData.data());
        }
    }
}
