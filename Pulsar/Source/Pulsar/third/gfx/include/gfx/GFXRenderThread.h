#pragma once
#include "GFXResourceManager.h"

#include <vector>
#include <mutex>
#include <functional>

namespace gfx
{
    // -------------------------------------------------------------------------
    // Render thread abstraction.
    //
    // Commands are expressed as lambdas capturing their data by value.
    // The lambda receives a GFXResourceManager& as its execution context.
    //
    // In single-threaded mode (current):
    //   - Enqueue() pushes a deferred lambda into a queue.
    //   - Execute() runs a lambda synchronously on the caller thread.
    //   - Flush() drains the deferred queue at a sync point (e.g. frame start).
    //
    // In future multi-threaded mode:
    //   - Enqueue() pushes into a lock-free queue consumed by the render thread.
    //   - Execute() may block until the render thread processes it.
    //   - Flush() becomes a no-op on the game thread; the render thread drains
    //     its queue automatically.
    // -------------------------------------------------------------------------
    class GFXRenderThread
    {
    public:
        using Command = std::function<void(GFXResourceManager&)>;

        explicit GFXRenderThread(GFXApplication* app);
        ~GFXRenderThread();

        // -----------------------------------------------------------------
        // Deferred submission (preferred when no immediate access needed)
        // -----------------------------------------------------------------
        void Enqueue(Command cmd);

        BufferHandle              EnqueueCreateBuffer(const GFXBufferDesc& desc);
        TextureHandle             EnqueueCreateTexture2D(const GFXTextureCreateDesc& desc);
        TextureHandle             EnqueueCreateTextureCube(int32_t size);
        TextureHandle             EnqueueCreateRenderTarget(const GFXTextureCreateDesc& desc);
        FrameBufferObjectHandle   EnqueueCreateFrameBufferObject(const array_list<GFXTexture2DView_sp>& attachments);
        GpuProgramHandle          EnqueueCreateGpuProgram(GFXGpuProgramStageFlags stage, const void* code, size_t length);
        DescriptorSetLayoutHandle EnqueueCreateDescriptorSetLayout(const std::vector<GFXDescriptorSetLayoutDesc>& bindings);
        VertexLayoutDescriptionHandle EnqueueCreateVertexLayoutDescription();

        void EnqueueDestroy(BufferHandle handle);
        void EnqueueDestroy(TextureHandle handle);
        void EnqueueDestroy(FrameBufferObjectHandle handle);
        void EnqueueDestroy(GpuProgramHandle handle);
        void EnqueueDestroy(DescriptorSetLayoutHandle handle);
        void EnqueueDestroy(VertexLayoutDescriptionHandle handle);

        void EnqueueUploadBuffer(BufferHandle handle, const void* data, size_t size);
        void EnqueueUploadTexture(TextureHandle handle, const void* data, uint32_t width, uint32_t height, GFXTextureFormat format);

        // -----------------------------------------------------------------
        // Immediate execution (only use when the caller needs the resource
        // right away, e.g. to Fill() a buffer or bind a pipeline)
        // -----------------------------------------------------------------
        void Execute(Command cmd);

        BufferHandle              CreateBufferImmediate(const GFXBufferDesc& desc);
        TextureHandle             CreateTexture2DImmediate(const GFXTextureCreateDesc& desc);
        TextureHandle             CreateTextureCubeImmediate(int32_t size);
        TextureHandle             CreateRenderTargetImmediate(const GFXTextureCreateDesc& desc);
        FrameBufferObjectHandle   CreateFrameBufferObjectImmediate(const array_list<GFXTexture2DView_sp>& attachments);
        GpuProgramHandle          CreateGpuProgramImmediate(GFXGpuProgramStageFlags stage, const void* code, size_t length);
        DescriptorSetLayoutHandle CreateDescriptorSetLayoutImmediate(const std::vector<GFXDescriptorSetLayoutDesc>& bindings);
        VertexLayoutDescriptionHandle CreateVertexLayoutDescriptionImmediate();

        void DestroyImmediate(BufferHandle handle);
        void DestroyImmediate(TextureHandle handle);
        void DestroyImmediate(FrameBufferObjectHandle handle);
        void DestroyImmediate(GpuProgramHandle handle);
        void DestroyImmediate(DescriptorSetLayoutHandle handle);
        void DestroyImmediate(VertexLayoutDescriptionHandle handle);

        void UploadBufferImmediate(BufferHandle handle, const void* data, size_t size);
        void UploadTextureImmediate(TextureHandle handle, const void* data, uint32_t width, uint32_t height, GFXTextureFormat format);

        // -----------------------------------------------------------------
        // Flush the deferred command queue.
        // Call this at a known sync point (e.g. begin of frame).
        // -----------------------------------------------------------------
        void Flush();

        // -----------------------------------------------------------------
        // Frame lifecycle
        // -----------------------------------------------------------------
        void BeginFrame(uint64_t frameIndex);
        void EndFrame(uint64_t frameIndex);

        // -----------------------------------------------------------------
        // Accessors
        // -----------------------------------------------------------------
        GFXResourceManager* GetResourceManager() const { return m_resourceManager.get(); }

    private:
        std::unique_ptr<GFXResourceManager> m_resourceManager;
        std::vector<Command> m_deferredQueue;

        // In multi-threaded mode this will be replaced by a lock-free queue.
        // For now a simple mutex protects the deferred queue.
        std::mutex m_queueMutex;
    };

} // namespace gfx
