#pragma once
#include "GFXResourceManager.h"
#include "GFXCommandBuffer.h"

#include <vector>
#include <mutex>
#include <functional>

namespace gfx
{
    // -------------------------------------------------------------------------
    // Command list abstraction.
    //
    // Hides GFXRenderThread from user code.  Provides both deferred
    // (Enqueue/Flush) and synchronous (Create/Destroy/Upload) operations.
    //
    // Render commands (CmdDraw, CmdBindGraphicsPipeline, …) are NOT duplicated
    // here; callers obtain the active GFXCommandBuffer via GetCommandBuffer()
    // and issue draw commands directly on it.
    // -------------------------------------------------------------------------
    class GFXCommandList
    {
    public:
        using Command = std::function<void(GFXCommandList&)>;

        virtual ~GFXCommandList() = default;

        // -----------------------------------------------------------------
        // Deferred submission
        // -----------------------------------------------------------------
        virtual void Enqueue(Command cmd) = 0;
        virtual void Flush() = 0;

        // -----------------------------------------------------------------
        // Frame lifecycle (forwarded to resource manager)
        // -----------------------------------------------------------------
        virtual void BeginFrame(uint64_t frameIndex) = 0;
        virtual void EndFrame(uint64_t frameIndex) = 0;

        // -----------------------------------------------------------------
        // Resource creation (synchronous — caller needs handle immediately)
        // -----------------------------------------------------------------
        virtual BufferHandle              CreateBuffer(const GFXBufferDesc& desc) = 0;
        virtual TextureHandle             CreateTexture2D(const GFXTextureCreateDesc& desc) = 0;
        virtual TextureHandle             CreateTextureCube(int32_t size) = 0;
        virtual TextureHandle             CreateRenderTarget(const GFXTextureCreateDesc& desc) = 0;
        virtual FrameBufferObjectHandle   CreateFrameBufferObject(const array_list<GFXTexture2DView_sp>& attachments) = 0;
        virtual GpuProgramHandle          CreateGpuProgram(GFXGpuProgramStageFlags stage, const void* code, size_t length) = 0;
        virtual DescriptorSetLayoutHandle CreateDescriptorSetLayout(const std::vector<GFXDescriptorSetLayoutDesc>& bindings) = 0;
        virtual VertexLayoutDescriptionHandle CreateVertexLayoutDescription() = 0;

        // -----------------------------------------------------------------
        // Resource destruction (synchronous API; actual GPU free is deferred
        // inside the resource manager until a safe frame)
        // -----------------------------------------------------------------
        virtual void Destroy(BufferHandle handle) = 0;
        virtual void Destroy(TextureHandle handle) = 0;
        virtual void Destroy(FrameBufferObjectHandle handle) = 0;
        virtual void Destroy(GpuProgramHandle handle) = 0;
        virtual void Destroy(DescriptorSetLayoutHandle handle) = 0;
        virtual void Destroy(VertexLayoutDescriptionHandle handle) = 0;

        // -----------------------------------------------------------------
        // Resource upload (can be used inside Enqueue or called directly)
        // -----------------------------------------------------------------
        virtual void UploadBuffer(BufferHandle handle, const void* data, size_t size) = 0;
        virtual void UploadTexture(TextureHandle handle, const void* data, uint32_t width, uint32_t height, GFXTextureFormat format) = 0;

        // -----------------------------------------------------------------
        // Active command buffer (for render commands)
        // -----------------------------------------------------------------
        GFXCommandBuffer* GetCommandBuffer() const { return m_commandBuffer; }
        void SetCommandBuffer(GFXCommandBuffer* cmdBuffer) { m_commandBuffer = cmdBuffer; }

    protected:
        GFXResourceManager* m_resourceManager = nullptr;
        GFXCommandBuffer*   m_commandBuffer   = nullptr;
    };

    // -------------------------------------------------------------------------
    // Immediate command list — Enqueue() executes synchronously on the caller
    // thread.  Flush() is a no-op.
    // -------------------------------------------------------------------------
    class GFXCommandListImmediate final : public GFXCommandList
    {
    public:
        explicit GFXCommandListImmediate(GFXResourceManager* resMgr);

        void Enqueue(Command cmd) override;
        void Flush() override;
        void BeginFrame(uint64_t frameIndex) override;
        void EndFrame(uint64_t frameIndex) override;

        BufferHandle              CreateBuffer(const GFXBufferDesc& desc) override;
        TextureHandle             CreateTexture2D(const GFXTextureCreateDesc& desc) override;
        TextureHandle             CreateTextureCube(int32_t size) override;
        TextureHandle             CreateRenderTarget(const GFXTextureCreateDesc& desc) override;
        FrameBufferObjectHandle   CreateFrameBufferObject(const array_list<GFXTexture2DView_sp>& attachments) override;
        GpuProgramHandle          CreateGpuProgram(GFXGpuProgramStageFlags stage, const void* code, size_t length) override;
        DescriptorSetLayoutHandle CreateDescriptorSetLayout(const std::vector<GFXDescriptorSetLayoutDesc>& bindings) override;
        VertexLayoutDescriptionHandle CreateVertexLayoutDescription() override;

        void Destroy(BufferHandle handle) override;
        void Destroy(TextureHandle handle) override;
        void Destroy(FrameBufferObjectHandle handle) override;
        void Destroy(GpuProgramHandle handle) override;
        void Destroy(DescriptorSetLayoutHandle handle) override;
        void Destroy(VertexLayoutDescriptionHandle handle) override;

        void UploadBuffer(BufferHandle handle, const void* data, size_t size) override;
        void UploadTexture(TextureHandle handle, const void* data, uint32_t width, uint32_t height, GFXTextureFormat format) override;
    };

    // -------------------------------------------------------------------------
    // Deferred command list — Enqueue() stores lambdas in an internal queue.
    // Flush() drains the queue and executes them on the caller thread.
    //
    // Creation/Destroy/Upload methods are synchronous (they touch the resource
    // manager directly) so that callers can use handles immediately.
    // -------------------------------------------------------------------------
    class GFXCommandListDeferred final : public GFXCommandList
    {
    public:
        explicit GFXCommandListDeferred(GFXResourceManager* resMgr);

        void Enqueue(Command cmd) override;
        void Flush() override;
        void BeginFrame(uint64_t frameIndex) override;
        void EndFrame(uint64_t frameIndex) override;

        BufferHandle              CreateBuffer(const GFXBufferDesc& desc) override;
        TextureHandle             CreateTexture2D(const GFXTextureCreateDesc& desc) override;
        TextureHandle             CreateTextureCube(int32_t size) override;
        TextureHandle             CreateRenderTarget(const GFXTextureCreateDesc& desc) override;
        FrameBufferObjectHandle   CreateFrameBufferObject(const array_list<GFXTexture2DView_sp>& attachments) override;
        GpuProgramHandle          CreateGpuProgram(GFXGpuProgramStageFlags stage, const void* code, size_t length) override;
        DescriptorSetLayoutHandle CreateDescriptorSetLayout(const std::vector<GFXDescriptorSetLayoutDesc>& bindings) override;
        VertexLayoutDescriptionHandle CreateVertexLayoutDescription() override;

        void Destroy(BufferHandle handle) override;
        void Destroy(TextureHandle handle) override;
        void Destroy(FrameBufferObjectHandle handle) override;
        void Destroy(GpuProgramHandle handle) override;
        void Destroy(DescriptorSetLayoutHandle handle) override;
        void Destroy(VertexLayoutDescriptionHandle handle) override;

        void UploadBuffer(BufferHandle handle, const void* data, size_t size) override;
        void UploadTexture(TextureHandle handle, const void* data, uint32_t width, uint32_t height, GFXTextureFormat format) override;

    private:
        std::vector<Command> m_queue;
        std::mutex m_mutex;
    };

} // namespace gfx
