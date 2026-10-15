#pragma once
#include <Pulsar/Rendering/RenderResourceRegistry.h>
#include <Pulsar/Rendering/ShaderPropertyLayout.h>

#include <gfx/GFXDescriptorSet.h>
#include <gfx/GFXResource.h>

#include <cstdint>
#include <map>
#include <memory>
#include <vector>

namespace pulsar
{
    // Global, content-addressed cache of descriptor sets — shared by every set (set0 material /
    // set1 perPass / set2 perDraw). A descriptor set is fully described by its content:
    //   { layout (globally de-duplicated), the ordered list of bound resource identities
    //     (GFXBuffer* / GFXTexture2DView* per reflected binding) }.
    // Two Resolve() calls with identical content resolve to the SAME set. Only *bindings* participate
    // in the key — cbuffer bytes and dynamic offsets do NOT (descriptors store the buffer handle,
    // not its bytes; dynamic offsets are supplied at bind time).
    //
    // Owners are stateless: they do not store the set or any handle. Each draw recomputes the key
    // and calls Resolve(); a hit returns the existing set and stamps it with the current frame. A set
    // Entries not reused for a couple of frames are reclaimed; the GPU-safety part of the
    // Reclaim is driven by the unified GPU timeline: an entry is dropped once the
    // submission that last referenced it has provably completed.
    //
    // This unifies "is the shader builtin / compiling / compiled?" away: the key is derived from
    // whatever program/resources are current, so content changes automatically pick a different
    // set — no explicit rebuild/dirty decision.
    class DescriptorSetCache
    {
    public:
        static DescriptorSetCache& Instance();

        struct Key
        {
            const gfx::GFXDescriptorSetLayout*  m_layout = nullptr;
            std::vector<const gfx::GFXResource*> m_bindings; // by reflected binding order

            bool operator<(const Key& o) const
            {
                if (m_layout != o.m_layout) return m_layout < o.m_layout;
                return m_bindings < o.m_bindings;
            }
        };

        // Look up (or build) the set for the given (de-duplicated) layout + reflected set layout +
        // resolved resources. The key is extracted from reg by resolving each reflected binding.
        // A null reflection means an empty set (layout with no bindings). Returns a set ready to
        // bind (its lifetime is managed by the cache — do not store it past the current frame;
        // re-Resolve() each frame instead).
        gfx::GFXDescriptorSet* Resolve(
            const gfx::GFXDescriptorSetLayoutPtr& layout,
            const ShaderPropertySetLayout* reflection,
            const RenderResourceRegistry& reg);

        // Reclaim sets whose last referencing submission has completed on the timeline.
        void Tick();

        // Drop every cached set/layout ref. Must run while the GFX device is still alive and the
        // GPU is idle (engine shutdown, after the render thread has drained) so the held
        // GFXDescriptorSetPtr destructors release Vulkan objects before vkDestroyDevice.
        void Clear();

    private:
        struct Entry
        {
            gfx::GFXDescriptorSetPtr m_set;
            uint64_t                 m_lastUsedFrame = 0;
        };

        std::map<Key, Entry> m_entries;
    };
}
