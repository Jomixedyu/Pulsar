#pragma once
#include "GFXResource.h"
#include "GFXHandle.h"
#include "GFXBuffer.h"
#include "GFXTexture.h"
#include "GFXGpuProgram.h"
#include "GFXCommandBuffer.h"

#include <vector>
#include <unordered_map>
#include <mutex>

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
        bool isPendingDestroy = false;
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
        // Handle allocation
        // -----------------------------------------------------------------
        template<typename HandleType>
        HandleType AllocHandle()
        {
            std::lock_guard<std::mutex> lock(m_mutex);
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
        void CreateDescriptorSetLayout(DescriptorSetLayoutHandle handle, const std::vector<GFXDescriptorSetLayoutDesc>& bindings);
        void CreateVertexLayoutDescription(VertexLayoutDescriptionHandle handle);

        void DestroyResource(uint32_t resourceId, uint16_t generation);

        void UploadBuffer(BufferHandle handle, const void* data, size_t size);
        void UploadTexture(TextureHandle handle, const void* data, uint32_t width, uint32_t height, GFXTextureFormat format);

        // -----------------------------------------------------------------
        // Frame lifecycle
        // -----------------------------------------------------------------
        void BeginFrame(uint64_t frameIndex);
        void EndFrame(uint64_t frameIndex);

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
        GFXVertexLayoutDescription* GetVertexLayoutDescription(VertexLayoutDescriptionHandle handle) const;

        // Generic lookup
        GFXResource* GetResource(uint32_t resourceId) const;

    private:
        // Internal helpers
        uint32_t AllocSlotInternal();
        void FreeSlot(uint32_t index);
        bool ValidateHandle(uint32_t index, uint16_t generation) const;
        void ProcessDeferredDestroys();

        // Resource storage (sparse array with free-list)
        std::vector<GFXResourceSlot> m_slots;
        std::vector<uint32_t> m_freeSlots;

        GFXApplication* m_app = nullptr;
        uint64_t m_currentFrame = 0;

        // Deferred destruction queue (resources destroyed N frames after queue)
        static constexpr uint32_t kDeferredDestroyFrames = 2;
        std::vector<std::pair<uint64_t, uint32_t>> m_deferredDestroyQueue;

        mutable std::mutex m_mutex;
    };

} // namespace gfx
