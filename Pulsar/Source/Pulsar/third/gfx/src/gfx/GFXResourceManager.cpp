#include "gfx/GFXResourceManager.h"
#include "gfx/GFXApplication.h"

namespace gfx
{
    GFXResourceManager::GFXResourceManager(GFXApplication* app)
        : m_app(app)
    {
    }

    GFXResourceManager::~GFXResourceManager()
    {
        // Ensure all deferred destroys are processed
        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto& slot : m_slots)
        {
            if (slot.resource)
            {
                slot.resource.reset();
            }
        }
    }

    // -------------------------------------------------------------------------
    // Synchronous creation
    // -------------------------------------------------------------------------
    static bool ValidateHandleForCreation(const std::vector<GFXResourceSlot>& slots, uint32_t index, uint16_t generation)
    {
        if (index >= slots.size()) return false;
        if (slots[index].generation != generation) return false;
        if (slots[index].isPendingDestroy) return false;
        return true;
    }

    void GFXResourceManager::CreateBuffer(BufferHandle handle, const GFXBufferDesc& desc)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!ValidateHandleForCreation(m_slots, handle.index, handle.generation)) return;
        auto gfxBuffer = m_app->CreateBuffer(desc);
        if (gfxBuffer)
        {
            gfxBuffer->SetResourceId(handle.index);
            m_slots[handle.index].resource = std::static_pointer_cast<GFXResource>(gfxBuffer);
        }
    }

    void GFXResourceManager::CreateTexture2D(TextureHandle handle, const GFXTextureCreateDesc& desc)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!ValidateHandleForCreation(m_slots, handle.index, handle.generation)) return;
        auto gfxTex = m_app->CreateTexture2DFromMemory(
            desc.ImageData, desc.DataLength,
            desc.Width, desc.Height,
            desc.Format, desc.SamplerCfg);
        if (gfxTex)
        {
            gfxTex->SetResourceId(handle.index);
            m_slots[handle.index].resource = std::static_pointer_cast<GFXResource>(gfxTex);
        }
    }

    void GFXResourceManager::CreateTextureCube(TextureHandle handle, int32_t size)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!ValidateHandleForCreation(m_slots, handle.index, handle.generation)) return;
        auto gfxTex = m_app->CreateTextureCube(size);
        if (gfxTex)
        {
            gfxTex->SetResourceId(handle.index);
            m_slots[handle.index].resource = std::static_pointer_cast<GFXResource>(gfxTex);
        }
    }

    void GFXResourceManager::CreateRenderTarget(TextureHandle handle, const GFXTextureCreateDesc& desc)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!ValidateHandleForCreation(m_slots, handle.index, handle.generation)) return;
        auto gfxTex = m_app->CreateRenderTarget(
            desc.Width, desc.Height, desc.TargetType,
            desc.Format, desc.SamplerCfg,
            desc.SampleCount, desc.IsTransientAttachment);
        if (gfxTex)
        {
            gfxTex->SetResourceId(handle.index);
            m_slots[handle.index].resource = std::static_pointer_cast<GFXResource>(gfxTex);
        }
    }

    void GFXResourceManager::CreateFrameBufferObject(FrameBufferObjectHandle handle, const array_list<GFXTexture2DView_sp>& attachments)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!ValidateHandleForCreation(m_slots, handle.index, handle.generation)) return;
        auto fbo = m_app->CreateFrameBufferObject(attachments);
        if (fbo)
        {
            fbo->SetResourceId(handle.index);
            m_slots[handle.index].resource = std::static_pointer_cast<GFXResource>(fbo);
        }
    }

    void GFXResourceManager::CreateGpuProgram(GpuProgramHandle handle, GFXGpuProgramStageFlags stage, const void* code, size_t length)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!ValidateHandleForCreation(m_slots, handle.index, handle.generation)) return;
        auto gfxProg = m_app->CreateGpuProgram(stage, static_cast<const uint8_t*>(code), length);
        if (gfxProg)
        {
            gfxProg->SetResourceId(handle.index);
            m_slots[handle.index].resource = std::static_pointer_cast<GFXResource>(gfxProg);
        }
    }

    void GFXResourceManager::CreateDescriptorSetLayout(DescriptorSetLayoutHandle handle, const std::vector<GFXDescriptorSetLayoutDesc>& bindings)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!ValidateHandleForCreation(m_slots, handle.index, handle.generation)) return;
        auto gfxLayout = m_app->CreateDescriptorSetLayout(bindings.data(), bindings.size());
        if (gfxLayout)
        {
            gfxLayout->SetResourceId(handle.index);
            m_slots[handle.index].resource = std::static_pointer_cast<GFXResource>(gfxLayout);
        }
    }

    void GFXResourceManager::CreateVertexLayoutDescription(VertexLayoutDescriptionHandle handle)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!ValidateHandleForCreation(m_slots, handle.index, handle.generation)) return;
        auto gfxLayout = m_app->CreateVertexLayoutDescription();
        if (gfxLayout)
        {
            gfxLayout->SetResourceId(handle.index);
            m_slots[handle.index].resource = std::static_pointer_cast<GFXResource>(gfxLayout);
        }
    }

    void GFXResourceManager::DestroyResource(uint32_t resourceId, uint16_t generation)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (ValidateHandle(resourceId, generation))
        {
            m_slots[resourceId].isPendingDestroy = true;
            m_deferredDestroyQueue.push_back({ m_currentFrame + kDeferredDestroyFrames, resourceId });
        }
    }

    void GFXResourceManager::UploadBuffer(BufferHandle handle, const void* data, size_t /*size*/)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto* buffer = GetBuffer(handle);
        if (buffer)
        {
            buffer->Fill(data);
        }
    }

    void GFXResourceManager::UploadTexture(TextureHandle handle, const void* /*data*/, uint32_t /*width*/, uint32_t /*height*/, GFXTextureFormat /*format*/)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto* texture = GetTexture(handle);
        if (texture)
        {
            // TODO: implement texture upload via gfx app
        }
    }

    // -------------------------------------------------------------------------
    // Lookups
    // -------------------------------------------------------------------------
    GFXBuffer* GFXResourceManager::GetBuffer(BufferHandle handle) const
    {
        if (!ValidateHandle(handle.index, handle.generation)) return nullptr;
        return static_cast<GFXBuffer*>(m_slots[handle.index].resource.get());
    }

    GFXTexture* GFXResourceManager::GetTexture(TextureHandle handle) const
    {
        if (!ValidateHandle(handle.index, handle.generation)) return nullptr;
        return static_cast<GFXTexture*>(m_slots[handle.index].resource.get());
    }

    GFXFrameBufferObject* GFXResourceManager::GetFrameBufferObject(FrameBufferObjectHandle handle) const
    {
        if (!ValidateHandle(handle.index, handle.generation)) return nullptr;
        return static_cast<GFXFrameBufferObject*>(m_slots[handle.index].resource.get());
    }

    GFXGpuProgram* GFXResourceManager::GetGpuProgram(GpuProgramHandle handle) const
    {
        if (!ValidateHandle(handle.index, handle.generation)) return nullptr;
        return static_cast<GFXGpuProgram*>(m_slots[handle.index].resource.get());
    }

    GFXDescriptorSetLayout* GFXResourceManager::GetDescriptorSetLayout(DescriptorSetLayoutHandle handle) const
    {
        if (!ValidateHandle(handle.index, handle.generation)) return nullptr;
        return static_cast<GFXDescriptorSetLayout*>(m_slots[handle.index].resource.get());
    }

    GFXVertexLayoutDescription* GFXResourceManager::GetVertexLayoutDescription(VertexLayoutDescriptionHandle handle) const
    {
        if (!ValidateHandle(handle.index, handle.generation)) return nullptr;
        return static_cast<GFXVertexLayoutDescription*>(m_slots[handle.index].resource.get());
    }

    GFXGpuProgram_sp GFXResourceManager::GetGpuProgramShared(GpuProgramHandle handle) const
    {
        return GetSharedPtr<GFXGpuProgram>(handle.index, handle.generation);
    }

    GFXDescriptorSetLayout_sp GFXResourceManager::GetDescriptorSetLayoutShared(DescriptorSetLayoutHandle handle) const
    {
        return GetSharedPtr<GFXDescriptorSetLayout>(handle.index, handle.generation);
    }

    GFXTexture_sp GFXResourceManager::GetTextureShared(TextureHandle handle) const
    {
        return GetSharedPtr<GFXTexture>(handle.index, handle.generation);
    }

    GFXFrameBufferObject_sp GFXResourceManager::GetFrameBufferObjectShared(FrameBufferObjectHandle handle) const
    {
        return GetSharedPtr<GFXFrameBufferObject>(handle.index, handle.generation);
    }

    GFXVertexLayoutDescription_sp GFXResourceManager::GetVertexLayoutDescriptionShared(VertexLayoutDescriptionHandle handle) const
    {
        return GetSharedPtr<GFXVertexLayoutDescription>(handle.index, handle.generation);
    }

    GFXResource* GFXResourceManager::GetResource(uint32_t resourceId) const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (resourceId >= m_slots.size()) return nullptr;
        return m_slots[resourceId].resource.get();
    }

    // -------------------------------------------------------------------------
    // Frame lifecycle
    // -------------------------------------------------------------------------
    void GFXResourceManager::BeginFrame(uint64_t frameIndex)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_currentFrame = frameIndex;
        ProcessDeferredDestroys();
    }

    void GFXResourceManager::EndFrame(uint64_t frameIndex)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_currentFrame = frameIndex;
        ProcessDeferredDestroys();
    }

    // -------------------------------------------------------------------------
    // Internal helpers
    // -------------------------------------------------------------------------
    uint32_t GFXResourceManager::AllocSlotInternal()
    {
        if (!m_freeSlots.empty())
        {
            uint32_t index = m_freeSlots.back();
            m_freeSlots.pop_back();
            m_slots[index].generation++;
            m_slots[index].isPendingDestroy = false;
            return index;
        }

        uint32_t index = static_cast<uint32_t>(m_slots.size());
        m_slots.emplace_back();
        m_slots[index].generation = 1;
        return index;
    }

    void GFXResourceManager::FreeSlot(uint32_t index)
    {
        if (index >= m_slots.size()) return;
        m_slots[index].resource.reset();
        m_slots[index].isPendingDestroy = false;
        m_freeSlots.push_back(index);
    }

    bool GFXResourceManager::ValidateHandle(uint32_t index, uint16_t generation) const
    {
        if (index >= m_slots.size()) return false;
        if (m_slots[index].generation != generation) return false;
        if (!m_slots[index].resource) return false;
        if (m_slots[index].isPendingDestroy) return false;
        return true;
    }

    void GFXResourceManager::ProcessDeferredDestroys()
    {
        auto it = m_deferredDestroyQueue.begin();
        while (it != m_deferredDestroyQueue.end())
        {
            if (it->first <= m_currentFrame)
            {
                FreeSlot(it->second);
                it = m_deferredDestroyQueue.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

} // namespace gfx
