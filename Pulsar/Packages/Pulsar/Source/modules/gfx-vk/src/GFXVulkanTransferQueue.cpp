#include "GFXVulkanTransferQueue.h"

#include "GFXVulkanApplication.h"
#include <gfx-vk/BufferHelper.h>

#include <cstring>

namespace gfx
{
    GFXVulkanTransferQueue::GFXVulkanTransferQueue(GFXVulkanApplication* app)
        : m_app(app)
    {
        m_slots.resize(GFXVulkanApplication::kFrameSlotCount);
    }

    GFXVulkanTransferQueue::~GFXVulkanTransferQueue()
    {
        for (auto& slot : m_slots)
            for (auto& chunk : slot.m_chunks)
                DestroyChunk(chunk);
    }

    GFXVulkanTransferQueue::Chunk GFXVulkanTransferQueue::CreateChunk(size_t capacity)
    {
        Chunk chunk;
        chunk.m_capacity = capacity;
        BufferHelper::CreateBuffer(m_app, capacity,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            chunk.m_buffer, chunk.m_memory);
        vkMapMemory(m_app->GetVkDevice(), chunk.m_memory, 0, capacity, 0, &chunk.m_mapped);
        return chunk;
    }

    void GFXVulkanTransferQueue::DestroyChunk(Chunk& chunk)
    {
        if (chunk.m_buffer == VK_NULL_HANDLE)
            return;
        vkUnmapMemory(m_app->GetVkDevice(), chunk.m_memory);
        BufferHelper::DestroyBuffer(m_app, chunk.m_buffer, chunk.m_memory);
        chunk = {};
    }

    std::pair<GFXVulkanTransferQueue::Chunk*, size_t> GFXVulkanTransferQueue::Allocate(size_t size)
    {
        const uint64_t frame = m_app->GetFrameCount();
        const uint32_t slotIndex = m_app->GetFrameInFlightIndex();

        // First allocation of a new frame: recycle this slot. The gfx frame counter only advances
        // on successful frames, so the previous occupant of this slot (frame - kFrameSlotCount)
        // is already done, while a skipped frame keeps the same slot and just keeps accumulating.
        if (frame != m_lastFrame)
        {
            Slot& fresh = m_slots[slotIndex];
            fresh.m_chunkCursor = 0;
            for (auto& chunk : fresh.m_chunks)
                chunk.m_offset = 0;
            m_lastFrame = frame;
        }

        Slot& slot = m_slots[slotIndex];
        const size_t aligned = (size + kAlignment - 1) & ~(kAlignment - 1);

        // Advance through already-allocated chunks looking for room.
        while (slot.m_chunkCursor < slot.m_chunks.size())
        {
            Chunk& chunk = slot.m_chunks[slot.m_chunkCursor];
            if (chunk.m_offset + aligned <= chunk.m_capacity)
            {
                size_t off = chunk.m_offset;
                chunk.m_offset += aligned;
                return {&chunk, off};
            }
            ++slot.m_chunkCursor;
        }

        // No room: append a new chunk (large enough for this request).
        const size_t cap = aligned > kDefaultChunkSize ? aligned : kDefaultChunkSize;
        slot.m_chunks.push_back(CreateChunk(cap));
        Chunk& chunk = slot.m_chunks.back();
        chunk.m_offset = aligned;
        return {&chunk, 0};
    }

    void GFXVulkanTransferQueue::PushTransfer(const void* data, size_t size,
        std::move_only_function<void(VkCommandBuffer, VkBuffer, VkDeviceSize)> record)
    {
        if (!data || size == 0)
            return;

        std::lock_guard<std::mutex> lock(m_mutex);
        auto [chunk, off] = Allocate(size);
        std::memcpy(static_cast<uint8_t*>(chunk->m_mapped) + off, data, size);

        m_pending.push_back(Pending{chunk->m_buffer, static_cast<VkDeviceSize>(off), std::move(record)});
    }

    void GFXVulkanTransferQueue::PushRecord(std::move_only_function<void(VkCommandBuffer)> record)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_pending.push_back(Pending{VK_NULL_HANDLE, 0,
            [r = std::move(record)](VkCommandBuffer cmd, VkBuffer, VkDeviceSize) mutable { r(cmd); }});
    }

    bool GFXVulkanTransferQueue::HasPending()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return !m_pending.empty();
    }

    void GFXVulkanTransferQueue::FlushInto(VkCommandBuffer cmd)
    {
        std::vector<Pending> pending;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            pending.swap(m_pending);
        }

        if (pending.empty())
            return;

        for (auto& p : pending)
            p.m_record(cmd, p.m_src, p.m_srcOffset);

        VkMemoryBarrier2 barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2;
        barrier.srcStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
        barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
        barrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;
        barrier.dstAccessMask = VK_ACCESS_2_UNIFORM_READ_BIT | VK_ACCESS_2_SHADER_READ_BIT;

        VkDependencyInfo depInfo{};
        depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        depInfo.memoryBarrierCount = 1;
        depInfo.pMemoryBarriers = &barrier;

        vkCmdPipelineBarrier2(cmd, &depInfo);
    }
}
