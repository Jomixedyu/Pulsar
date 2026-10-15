#pragma once
#include "PrimitiveStruct.h"
#include <CoreLib/Type.h>
#include <gfx/GFXBuffer.h>
#include <gfx/GFXDescriptorSet.h>

namespace pulsar
{
    using jxcorlib::array_list;

    class PerRenderObjectDataManager
    {
    public:
        static constexpr uint32_t kInitialCapacity = 4096;
        static constexpr uint32_t kInvalidSlot = UINT32_MAX;

        PerRenderObjectDataManager() = default;
        ~PerRenderObjectDataManager() = default;

        void Initialize();
        void Destroy();

        uint32_t AllocSlot();
        void FreeSlot(uint32_t slot);

        void SetData(uint32_t slot, const PerRenderObjectData& data);

        void BeginFrame();
        void EndFrame();

        gfx::GFXBufferPtr GetBuffer() const { return m_buffer; }
        gfx::GFXBuffer* GetBufferRaw() const { return m_buffer.get(); }
        gfx::GFXDescriptorSetPtr GetDummyExtraSet() const { return m_dummyExtraSet; }
        gfx::GFXDescriptorSetLayoutPtr GetDummyExtraLayout() const { return m_dummyExtraLayout; }

    private:
        void Grow(uint32_t newCapacity);

        gfx::GFXBufferPtr m_buffer;
        gfx::GFXDescriptorSetPtr m_dummyExtraSet;
        gfx::GFXDescriptorSetLayoutPtr m_dummyExtraLayout;

        array_list<PerRenderObjectData> m_cpuData;
        array_list<bool> m_slotUsed;
        array_list<uint32_t> m_freeSlots;
        uint32_t m_nextSlot = 0;
        uint32_t m_capacity = 0;
    };
}
