#include "Rendering/DescriptorSetCache.h"

#include "Rendering/DescriptorSetAssembler.h"
#include <Pulsar/Application.h>

#include <utility>

namespace pulsar
{
    DescriptorSetCache& DescriptorSetCache::Instance()
    {
        static DescriptorSetCache s_instance;
        return s_instance;
    }

    gfx::GFXDescriptorSet* DescriptorSetCache::Resolve(
        const gfx::GFXDescriptorSetLayoutPtr& layout,
        const ShaderPropertySetLayout* reflection,
        const RenderResourceRegistry& reg)
    {
        if (!layout)
            return nullptr;

        // Build the content key: each reflected binding resolved to its live resource identity
        // (the same resource the assembler will bind, incl. gfx built-in fallbacks).
        Key key;
        key.m_layout = layout.get();
        if (reflection)
        {
            key.m_bindings.reserve(reflection->m_bindings.size());
            for (const auto& binding : reflection->m_bindings)
                key.m_bindings.push_back(reg.Resolve(binding));
        }

        auto [it, inserted] = m_entries.try_emplace(std::move(key));
        Entry& entry = it->second;

        if (inserted)
        {
            entry.m_set = layout->AllocateSet();
            if (reflection)
                DescriptorSetAssembler::Write(entry.m_set.get(), *reflection, reg);
        }

        // Stamp with the submission being recorded this frame; the entry is reclaimable
        // once the completed timeline has passed it.
        entry.m_lastUsedFrame = Application::GetGfxApp()->GetFrameCount();
        return entry.m_set.get();
    }

    void DescriptorSetCache::Tick()
    {
        // Reclaim sets whose last referencing submission has completed on the GPU
        // timeline (strictly greater: the stamp is the frame being recorded, whose
        // submission is stamp+1).
        const uint64_t completed = Application::GetGfxApp()->GetCompletedTimeline();
        for (auto it = m_entries.begin(); it != m_entries.end();)
        {
            if (completed > it->second.m_lastUsedFrame)
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
