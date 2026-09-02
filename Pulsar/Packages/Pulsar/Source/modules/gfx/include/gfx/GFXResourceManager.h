#pragma once
#include "GFXResource.h"
#include "GFXHandle.h"
#include "GFXBuffer.h"
#include "GFXTexture.h"
#include "GFXGpuProgram.h"
#include "GFXCommandBuffer.h"

#include <vector>
#include <unordered_map>
#include <shared_mutex>

namespace gfx
{
    // Forward declarations
    class GFXApplication;

    // -------------------------------------------------------------------------
    // Typed resource slot used internally by the manager.
    // -------------------------------------------------------------------------
    struct GFXResourceSlot
    {
        GFXResource_sp resource;      // nullptr if free
        uint16_t generation = 1;      // bumped on alloc, checked on lookup
    };

    // -------------------------------------------------------------------------
    // Resource manager: owns all GPU resources via typed handles.
    // All mutating operations are queued and flushed together.
    // -------------------------------------------------------------------------
    class GFXResourceManager
    {
    public:
        explicit GFXResourceManager(GFXApplication* app);
        ~GFXResourceManager();

        // -----------------------------------------------------------------
        // Handle allocation. Thread-safe: callable from any thread.
        // -----------------------------------------------------------------
        template<typename HandleType>
        HandleType AllocHandle()
        {
            std::lock_guard<std::shared_mutex> lock(m_mutex);
            uint32_t index = AllocSlotInternal();
            HandleType handle;
            handle.index = index;
            handle.generation = m_slots[index].generation;
            return handle;
        }

        // -----------------------------------------------------------------
        // Synchronous resource creation (called from RenderThread lambdas)
        // -----------------------------------------------------------------
        void CreateBuffer(BufferHandle handle, const GFXBufferDesc& desc);
        void CreateTexture2D(TextureHandle handle, const GFXTextureCreateDesc& desc);
        void CreateTextureCube(TextureHandle handle, int32_t size);
        void CreateRenderTarget(TextureHandle handle, const GFXTextureCreateDesc& desc);
        void CreateFrameBufferObject(FrameBufferObjectHandle handle, const array_list<GFXTexture2DView_sp>& attachments);
        void CreateGpuProgram(GpuProgramHandle handle, GFXGpuProgramStageFlags stage, const void* code, size_t length);
        void CreateDescriptorSetLayout(DescriptorSetLayoutHandle handle, const std::vector<GFXDescriptorLayoutDesc>& bindings);

        void DestroyResource(uint32_t resourceId, uint16_t generation);

        // Type-safe destruction: accepts any handle directly.
        template<typename T>
        void Destroy(GFXHandle<T> handle)
        {
            DestroyResource(handle.index, handle.generation);
        }

        void UploadBuffer(BufferHandle handle, const void* data, size_t size);
        void UploadTexture(TextureHandle handle, const void* data, uint32_t width, uint32_t height, GFXTextureFormat format);

        // -----------------------------------------------------------------
        // Immediate lookups (read-only). Used by the render thread or
        // during flush to resolve handles into actual resources.
        // Returns nullptr if the handle is stale or invalid.
        // -----------------------------------------------------------------
        GFXBuffer*                GetBuffer(BufferHandle handle) const;
        GFXTexture*               GetTexture(TextureHandle handle) const;
        GFXFrameBufferObject*     GetFrameBufferObject(FrameBufferObjectHandle handle) const;
        GFXGpuProgram*            GetGpuProgram(GpuProgramHandle handle) const;
        GFXDescriptorSetLayout*   GetDescriptorSetLayout(DescriptorSetLayoutHandle handle) const;

        // -----------------------------------------------------------------
        // Shared-pointer lookups (for interop with legacy APIs)
        // -----------------------------------------------------------------
        template<typename T>
        std::shared_ptr<T> GetSharedPtr(uint32_t index, uint16_t generation) const
        {
            std::shared_lock<std::shared_mutex> lock(m_mutex);
            if (index >= m_slots.size()) return nullptr;
            if (m_slots[index].generation != generation) return nullptr;
            return std::static_pointer_cast<T>(m_slots[index].resource);
        }

        GFXGpuProgram_sp            GetGpuProgramShared(GpuProgramHandle handle) const;
        GFXDescriptorSetLayout_sp   GetDescriptorSetLayoutShared(DescriptorSetLayoutHandle handle) const;
        GFXTexture_sp               GetTextureShared(TextureHandle handle) const;
        GFXFrameBufferObject_sp     GetFrameBufferObjectShared(FrameBufferObjectHandle handle) const;

        // Generic lookup
        GFXResource* GetResource(uint32_t resourceId) const;

    private:
        // Internal helpers
        uint32_t AllocSlotInternal();
        void FreeSlot(uint32_t index);
        bool ValidateHandle(uint32_t index, uint16_t generation) const;
        // Resource storage (sparse array with free-list)
        std::vector<GFXResourceSlot> m_slots;
        std::vector<uint32_t> m_freeSlots;

        GFXApplication* m_app = nullptr;

        mutable std::shared_mutex m_mutex;
    };

} // namespace gfx
