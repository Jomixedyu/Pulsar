#include "GFXVulkanBufferUploadQueue.h"

#include "GFXVulkanApplication.h"
#include "GFXVulkanBuffer.h"
#include <gfx-vk/BufferHelper.h>

#include <cstring>

namespace gfx
{
    GFXVulkanBufferUploadQueue::GFXVulkanBufferUploadQueue(GFXVulkanApplication* app)
        : m_app(app)
    {
        m_slots.resize(GFXVulkanApplication::MAX_FRAMES_IN_FLIGHT);
    }

    GFXVulkanBufferUploadQueue::~GFXVulkanBufferUploadQueue()
    {
        for (auto& slot : m_slots)
            for (auto& chunk : slot.m_chunks)
                DestroyChunk(chunk);
    }

    GFXVulkanBufferUploadQueue::Chunk GFXVulkanBufferUploadQueue::CreateChunk(size_t capacity)
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

    void GFXVulkanBufferUploadQueue::DestroyChunk(Chunk& chunk)
    {
        if (chunk.m_buffer == VK_NULL_HANDLE)
            return;
        vkUnmapMemory(m_app->GetVkDevice(), chunk.m_memory);
        BufferHelper::DestroyBuffer(m_app, chunk.m_buffer, chunk.m_memory);
        chunk = {};
    }

    std::pair<GFXVulkanBufferUploadQueue::Chunk*, size_t> GFXVulkanBufferUploadQueue::Allocate(size_t size)
    {
        const uint64_t frame = m_app->GetFrameCount();
        const uint32_t slotIndex = m_app->GetFrameInFlightIndex();

        // First allocation of a new frame: recycle this slot. The gfx frame counter only advances
        // on successful frames, so the previous occupant of this slot (frame - MAX_FRAMES_IN_FLIGHT)
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

    void GFXVulkanBufferUploadQueue::RequestUpload(GFXVulkanBuffer* dst, const void* data, size_t size, size_t dstOffset)
    {
        if (!dst || !data || size == 0)
            return;

        auto [chunk, off] = Allocate(size);
        std::memcpy(static_cast<uint8_t*>(chunk->m_mapped) + off, data, size);

        m_pending.push_back(Pending{
            chunk->m_buffer, static_cast<VkDeviceSize>(off),
            dst->GetVkBuffer(), static_cast<VkDeviceSize>(dstOffset), static_cast<VkDeviceSize>(size)});
    }

    void GFXVulkanBufferUploadQueue::FlushInto(VkCommandBuffer cmd)
    {
        if (m_pending.empty())
            return;

        for (const auto& p : m_pending)
        {
            VkBufferCopy region{};
            region.srcOffset = p.m_srcOffset;
            region.dstOffset = p.m_dstOffset;
            region.size = p.m_size;
            vkCmdCopyBuffer(cmd, p.m_src, p.m_dst, 1, &region);
        }

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

        m_pending.clear();
    }
}
