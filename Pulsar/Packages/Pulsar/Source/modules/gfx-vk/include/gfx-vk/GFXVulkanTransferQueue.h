#pragma once
#include <vulkan/vulkan.h>

#include <cstddef>
#include <cstdint>
#include <functional>
#include <mutex>
#include <utility>
#include <vector>

namespace gfx
{
    class GFXVulkanApplication;

    // Ring staging + batched frame-head transfers.
    //
    // The active staging slot is derived from the gfx frame counter
    // (GFXVulkanApplication::GetFrameInFlightIndex()); callers bump-allocate a slice
    // (CPU memcpy into host-visible staging) and supply a record callback that writes
    // its own copy/transition commands into the frame-head transfer command buffer.
    // The queue only owns staging and scheduling; it knows nothing about the
    // destination resources.
    //
    // The slot is recycled lazily: the first transfer of a new frame resets that
    // slot's bump cursor. Because the frame counter only advances on successful
    // frames, a skipped/early-out frame keeps accumulating into (and never clobbers)
    // the same slot until it is flushed.
    class GFXVulkanTransferQueue
    {
    public:
        GFXVulkanTransferQueue(GFXVulkanApplication* app);
        ~GFXVulkanTransferQueue();

        GFXVulkanTransferQueue(const GFXVulkanTransferQueue&) = delete;
        GFXVulkanTransferQueue& operator=(const GFXVulkanTransferQueue&) = delete;

        // Stage `size` bytes and enqueue a transfer recorded at the frame-head
        // flush; `record` receives (cmd, staging buffer, byte offset into it).
        // Callable from any thread.
        void PushTransfer(const void* data, size_t size,
            std::move_only_function<void(VkCommandBuffer, VkBuffer, VkDeviceSize)> record);

        // Enqueue a record-only transfer (layout transitions, barriers) with no
        // staged payload. Callable from any thread.
        void PushRecord(std::move_only_function<void(VkCommandBuffer)> record);

        // Record all pending transfers + one barrier into `cmd`; clears the list.
        void FlushInto(VkCommandBuffer cmd);

        bool HasPending();

    private:
        struct Chunk
        {
            VkBuffer m_buffer = VK_NULL_HANDLE;
            VkDeviceMemory m_memory = VK_NULL_HANDLE;
            void* m_mapped = nullptr;
            size_t m_capacity = 0;
            size_t m_offset = 0;
        };
        struct Slot
        {
            std::vector<Chunk> m_chunks;
            size_t m_chunkCursor = 0; // index of the chunk currently being filled
        };
        struct Pending
        {
            VkBuffer m_src;
            VkDeviceSize m_srcOffset;
            std::move_only_function<void(VkCommandBuffer, VkBuffer, VkDeviceSize)> m_record;
        };

        Chunk CreateChunk(size_t capacity);
        void DestroyChunk(Chunk& chunk);
        // Reserve `size` bytes in the current frame's slot, returning the owning chunk + byte offset.
        // Recycles the slot on the first call of a new frame.
        std::pair<Chunk*, size_t> Allocate(size_t size);

    private:
        GFXVulkanApplication* m_app = nullptr;
        std::vector<Slot> m_slots;
        uint64_t m_lastFrame = ~0ull; // frame counter value the active slot was last recycled for
        std::vector<Pending> m_pending;
        std::mutex m_mutex;

        static constexpr size_t kDefaultChunkSize = size_t(1) << 20; // 1 MiB
        static constexpr size_t kAlignment = 16;
    };
}
