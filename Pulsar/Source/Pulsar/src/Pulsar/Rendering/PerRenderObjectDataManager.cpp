#include "PerRenderObjectDataManager.h"
#include <Pulsar/Application.h>

namespace pulsar
{
    void PerRenderObjectDataManager::Initialize()
    {
        if (m_buffer) return; // already initialized

        Grow(kInitialCapacity);

        // Create dummy extra set (set2 for renderers without extra data)
        gfx::GFXDescriptorSetLayoutDesc dummyDesc{};
        m_dummyExtraLayout = Application::GetGfxApp()->CreateDescriptorSetLayout(&dummyDesc, 0);
        m_dummyExtraSet = Application::GetGfxApp()->GetDescriptorManager()->GetDescriptorSet(m_dummyExtraLayout);
        m_dummyExtraSet->Submit();
    }

    void PerRenderObjectDataManager::Destroy()
    {
        if (!m_buffer) return; // already destroyed
        m_dummyExtraSet.reset();
        m_dummyExtraLayout.reset();
        m_buffer.reset();
        m_cpuData.clear();
        m_slotUsed.clear();
        m_freeSlots.clear();
        m_nextSlot = 0;
        m_capacity = 0;
    }

    void PerRenderObjectDataManager::Grow(uint32_t newCapacity)
    {
        if (newCapacity <= m_capacity) return;

        gfx::GFXBufferDesc desc{};
        desc.Usage = gfx::GFXBufferUsage::ConstantBuffer;
        desc.StorageType = gfx::GFXBufferMemoryPosition::VisibleOnHost;
        desc.BufferSize = newCapacity * sizeof(PerRenderObjectData);
        desc.ElementSize = sizeof(PerRenderObjectData);
        auto newBuffer = Application::GetGfxApp()->CreateBuffer(desc);

        // Copy existing data if any
        if (m_buffer && m_capacity > 0)
        {
            // TODO: copy old data to new buffer
            // For now, simple approach: re-upload all cpu data
        }

        m_buffer = newBuffer;
        m_cpuData.resize(newCapacity);
        m_slotUsed.resize(newCapacity, false);
        m_capacity = newCapacity;
    }

    uint32_t PerRenderObjectDataManager::AllocSlot()
    {
        if (!m_buffer) Initialize();

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
        if (!m_buffer || m_nextSlot == 0) return;

        // Upload all active slots in one Fill
        size_t uploadSize = m_nextSlot * sizeof(PerRenderObjectData);
        m_buffer->Fill(m_cpuData.data());
    }
}
