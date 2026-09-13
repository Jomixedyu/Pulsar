#pragma once

#include "GFXBuiltinResources.h"
#include "GFXBuffer.h"
#include "GFXDescriptorSet.h"
#include "GFXFrameBufferObject.h"
#include "GFXGpuProgram.h"
#include "GFXResource.h"
#include "GFXSampler.h"
#include "GFXTexture.h"

#include <cstdint>
#include <initializer_list>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#if !defined(NDEBUG)
#include <stacktrace>
#endif

namespace gfx
{
    class GFXApplication;

    class GFXResourceRegistry
    {
    public:
        using ResourceId = GFXResource::ResourceId;
        static constexpr ResourceId InvalidResourceId = GFXResource::InvalidResourceId;

        virtual ~GFXResourceRegistry();

        // Lifecycle hooks, called by the backend while the device is alive:
        // Initialize after construction, Terminate before destruction. Builtin
        // resources are created/released here; they go through the virtual
        // Create* functions, so this must not run in a constructor/destructor.
        virtual void Initialize();
        virtual void Terminate();

        virtual GFXBufferPtr CreateBuffer(const GFXBufferDesc& desc) = 0;
        virtual GFXTexturePtr CreateTexture2D(const GFXTextureCreateDesc& desc) = 0;
        virtual GFXTexturePtr CreateTexture2DFromMemory(
            const uint8_t* imageData, size_t length,
            int width, int height,
            GFXTextureFormat format,
            const GFXSamplerConfig& samplerConfig) = 0;
        virtual GFXTexturePtr CreateTextureCube(int32_t size) = 0;
        virtual GFXTexturePtr CreateRenderTarget(
            int32_t width, int32_t height, GFXTextureTargetType type,
            GFXTextureFormat format, const GFXSamplerConfig& samplerCfg,
            uint32_t sampleCount = 1, bool isTransientAttachment = false) = 0;
        virtual GFXFrameBufferObjectPtr CreateFrameBufferObject(
            const array_list<GFXTexture2DViewPtr>& attachments) = 0;
        virtual GFXGpuProgramPtr CreateGpuProgram(
            GFXGpuProgramStageFlags stage, const void* code, size_t length) = 0;
        virtual GFXDescriptorSetLayoutPtr CreateDescriptorSetLayout(
            const GFXDescriptorLayoutDesc* layouts, size_t layoutCount) = 0;
        virtual GFXSamplerPtr CreateSampler(const GFXSamplerConfig& config) = 0;

        GFXDescriptorSetLayoutPtr GetOrCreateDescriptorSetLayout(
            const GFXDescriptorLayoutDesc* layouts, size_t layoutCount);
        GFXDescriptorSetLayoutPtr GetOrCreateDescriptorSetLayout(
            std::initializer_list<GFXDescriptorLayoutDesc> layouts)
        {
            return GetOrCreateDescriptorSetLayout(layouts.begin(), layouts.size());
        }

        GFXResource* FindResourceById(ResourceId id) const;

        virtual GFXApplication* GetApplication() const = 0;

        // Called by GFXApplication with the backend-owned timeline.
        void Tick(uint64_t currentTimeline);
        void FlushDestroyed();

        // Builtin fallback resources bound when a reflected descriptor has no
        // matching resource. Created by Initialize(), released by Terminate().
        GFXBuiltinResources& GetBuiltinResources() { return m_builtinResources; }

    protected:
        template<typename T>
        std::shared_ptr<T> RegisterResource(T* resource)
        {
#if !defined(NDEBUG)
            // Skip the registry's own frames; a few frames are enough
            // to tell where the resource was created.
            resource->SetCreationDebugInfo(std::to_string(std::stacktrace::current(2, 6)));
#endif
            m_activeResources.emplace(resource->GetResourceId(), resource);

            return std::shared_ptr<T>(resource, &GFXResourceRegistry::DestroyResource);
        }

        void QueueDestroyed(GFXResource* resource);
        static void DestroyResource(GFXResource* resource);

    private:
        uint64_t m_lastTickTimeline = 0;
        GFXBuiltinResources m_builtinResources;
        std::unordered_map<ResourceId, GFXResource*> m_activeResources;
        std::multimap<uint64_t, std::unique_ptr<GFXResource>> m_deferredDestroys;
        std::unordered_map<std::string, GFXDescriptorSetLayoutPtr> m_layoutCache;
        std::mutex m_layoutCacheMutex;
    };
}
