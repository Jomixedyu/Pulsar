#include "PerRenderObjectDataManager.h"
#include <Pulsar/Application.h>
#include <gfx/GFXResourceRegistry.h>

namespace pulsar
{
    void PerRenderObjectDataManager::Initialize()
    {
        if (m_buffer) return; // already initialized

        Grow(kInitialCapacity);

        // Create dummy extra set (set2 for renderers without extra data)
        gfx::GFXDescriptorLayoutDesc dummyDesc{};
        m_dummyExtraLayout = Application::GetGfxApp()->GetResourceRegistry()->GetOrCreateDescriptorSetLayout(&dummyDesc, 0);
        m_dummyExtraSet = m_dummyExtraLayout->AllocateSet();
        m_dummyExtraSet->Submit();
    }

    void PerRenderObjectDataManager::Destroy()
    {
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
        auto newBuffer = Application::GetGfxApp()->GetResourceRegistry()->CreateBuffer(desc);

        // Copy existing data if any
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
        m_buffer->Update(m_cpuData.data());
    }
}
