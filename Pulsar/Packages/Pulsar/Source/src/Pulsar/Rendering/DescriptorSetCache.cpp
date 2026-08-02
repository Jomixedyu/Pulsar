#include "Rendering/DescriptorSetCache.h"

#include "Rendering/DescriptorSetAssembler.h"

#include <utility>

namespace pulsar
{
    DescriptorSetCache& DescriptorSetCache::Instance()
    {
        static DescriptorSetCache s_instance;
        return s_instance;
    }

    gfx::GFXDescriptorSet* DescriptorSetCache::Get(
        const gfx::GFXDescriptorSetLayout_sp& layout,
        const ShaderPropertySetLayout& reflection,
        const RenderResourceRegistry& reg)
    {
        if (!layout)
            return nullptr;

        // Build the content key: each reflected binding resolved to its live resource identity
        // (the same resource the assembler will bind, incl. gfx built-in fallbacks).
        Key key;
        key.m_layout = layout.get();
        key.m_bindings.reserve(reflection.m_bindings.size());
        for (const auto& binding : reflection.m_bindings)
            key.m_bindings.push_back(reg.Resolve(binding));

        auto [it, inserted] = m_entries.try_emplace(std::move(key));
        Entry& entry = it->second;

        if (inserted)
        {
            entry.m_set = layout->AllocateSet();
            DescriptorSetAssembler::Write(entry.m_set.get(), reflection, reg);
        }

        entry.m_lastUsedFrame = m_frame;
        return entry.m_set.get();
    }

    void DescriptorSetCache::Tick()
    {
        ++m_frame;

        // Reclaim sets untouched for at least kGraceFrames (the GPU no longer references them under
        // the current per-frame vkQueueWaitIdle).
        for (auto it = m_entries.begin(); it != m_entries.end();)
        {
            if (m_frame - it->second.m_lastUsedFrame > kGraceFrames)
                it = m_entries.erase(it);
            else
                ++it;
        }
    }

    void DescriptorSetCache::Clear()
    {
        m_entries.clear();
    }
}
