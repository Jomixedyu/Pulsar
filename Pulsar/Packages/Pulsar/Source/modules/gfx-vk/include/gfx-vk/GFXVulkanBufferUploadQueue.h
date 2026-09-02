#pragma once
#include <vulkan/vulkan.h>

#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

namespace gfx
{
    class GFXVulkanApplication;
    class GFXVulkanBuffer;

    // Ring staging + batched device-local upload.
    //
    // The active staging slot is derived from the gfx frame counter
    // (GFXVulkanApplication::GetFrameInFlightIndex()); callers bump-allocate a slice via
    // RequestUpload (CPU memcpy into host-visible staging). FlushInto records all pending
    // vkCmdCopyBuffer into the frame's command buffer at frame head, followed by a single
    // transfer-write -> shader-read barrier. No per-update submit/wait (unlike GFXVulkanBuffer::Update
    // on device-local, which submits + vkQueueWaitIdle every call).
    //
    // The slot is recycled lazily: the first RequestUpload of a new frame resets that slot's bump
    // cursor. Because the frame counter only advances on successful frames, a skipped/early-out
    // frame keeps accumulating into (and never clobbers) the same slot until it is flushed.
    class GFXVulkanBufferUploadQueue
    {
    public:
        GFXVulkanBufferUploadQueue(GFXVulkanApplication* app);
        ~GFXVulkanBufferUploadQueue();

        GFXVulkanBufferUploadQueue(const GFXVulkanBufferUploadQueue&) = delete;
        GFXVulkanBufferUploadQueue& operator=(const GFXVulkanBufferUploadQueue&) = delete;

        // Copy `size` bytes from `data` into the current staging slot and enqueue a device copy
        // into `dst` at `dstOffset`.
        void RequestUpload(GFXVulkanBuffer* dst, const void* data, size_t size, size_t dstOffset = 0);

        // Record all pending copies + one barrier into `cmd`; clears the pending list.
        void FlushInto(VkCommandBuffer cmd);

        bool HasPending() const { return !m_pending.empty(); }

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
            VkBuffer m_dst;
            VkDeviceSize m_dstOffset;
            VkDeviceSize m_size;
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

        static constexpr size_t kDefaultChunkSize = size_t(1) << 20; // 1 MiB
        static constexpr size_t kAlignment = 16;
    };
}
