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
    // Two Get() calls with identical content resolve to the SAME set. Only *bindings* participate
    // in the key — cbuffer bytes and dynamic offsets do NOT (descriptors store the buffer handle,
    // not its bytes; dynamic offsets are supplied at bind time).
    //
    // Owners are stateless: they do not store the set or any handle. Each draw recomputes the key
    // and calls Get(); a hit returns the existing set and stamps it with the current frame. A set
    // that has not been used for GRACE frames is guaranteed no longer referenced by the GPU
    // (one full frame of grace under the current per-frame vkQueueWaitIdle) and is reclaimed.
    // §6 in-flight swaps the frame counter for a real timeline value; the interface is unchanged.
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
        // re-Get() each frame instead).
        gfx::GFXDescriptorSet* Get(
            const gfx::GFXDescriptorSetLayout_sp& layout,
            const ShaderPropertySetLayout* reflection,
            const RenderResourceRegistry& reg);

        // Advance the frame and reclaim sets untouched for GRACE frames.
        void Tick();

        // Drop every cached set/layout ref. Must run while the GFX device is still alive and the
        // GPU is idle (engine shutdown, after the render thread has drained) so the held
        // GFXDescriptorSet_sp destructors release Vulkan objects before vkDestroyDevice.
        void Clear();

    private:
        struct Entry
        {
            gfx::GFXDescriptorSet_sp m_set;
            uint64_t                 m_lastUsedFrame = 0;
        };

        static constexpr uint64_t kGraceFrames = 2;

        std::map<Key, Entry> m_entries;
        uint64_t             m_frame = 0;
    };
}
