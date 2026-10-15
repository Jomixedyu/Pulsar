#include <gfx/GFXResourceRegistry.h>
#include <gfx/GFXApplication.h>

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <iostream>
#include <limits>
#include <sstream>
#include <vector>

namespace gfx
{
    static const char* ToString(GFXResourceType type)
    {
        switch (type)
        {
        case GFXResourceType::Buffer: return "Buffer";
        case GFXResourceType::Texture: return "Texture";
        case GFXResourceType::TextureView: return "TextureView";
        case GFXResourceType::FrameBufferObject: return "FrameBufferObject";
        case GFXResourceType::GraphicsPipeline: return "GraphicsPipeline";
        case GFXResourceType::ComputePipeline: return "ComputePipeline";
        case GFXResourceType::DescriptorSetLayout: return "DescriptorSetLayout";
        case GFXResourceType::DescriptorSet: return "DescriptorSet";
        case GFXResourceType::GpuProgram: return "GpuProgram";
        case GFXResourceType::VertexLayoutDescription: return "VertexLayoutDescription";
        case GFXResourceType::CommandBuffer: return "CommandBuffer";
        case GFXResourceType::Swapchain: return "Swapchain";
        case GFXResourceType::RenderPass: return "RenderPass";
        case GFXResourceType::Sampler: return "Sampler";
        default: return "Unknown";
        }
    }

    GFXResourceRegistry::~GFXResourceRegistry()
    {
#if !defined(NDEBUG)
        std::unique_lock<std::mutex> dumpLock(m_resourcesMutex);
        if (!m_activeResources.empty())
        {
            std::map<std::string, size_t> counts;
            for (const auto& [id, resource] : m_activeResources)
            {
                ++counts[ToString(resource->GetResourceType())];
            }

            std::ostringstream message;
            message << "[GFXResourceRegistry] " << m_activeResources.size()
                    << " GFX resource(s) still active:";
            for (const auto& [type, count] : counts)
                message << ' ' << type << '=' << count;
            message << '\n';

            for (const auto& [id, resource] : m_activeResources)
            {
                message << "  " << ToString(resource->GetResourceType())
                        << " id=" << id
                        << " ptr=0x" << std::hex << reinterpret_cast<uintptr_t>(resource)
                        << std::dec << '\n'
                        << (resource->GetCreationDebugInfo().empty()
                            ? "<creation stack unavailable>"
                            : resource->GetCreationDebugInfo()) << '\n';
            }

            std::cerr << message.str();
        }
        dumpLock.unlock();
#endif
        // The backend must Terminate() the registry before destruction: every
        // resource released and the deferred queue flushed while the device is
        // alive. Anything left here is a lifecycle bug; crash in debug builds.
        assert(m_activeResources.empty() && "GFX resources still active at registry destruction; see the dump above");
        assert(m_deferredDestroys.empty() && "deferred destroys pending at registry destruction; call Terminate() before deleting the registry");

        {
            std::lock_guard<std::mutex> lock(m_layoutCacheMutex);
            m_layoutCache.clear();
        }
        m_deferredDestroys.clear();
        m_activeResources.clear();
    }

    void GFXResourceRegistry::Initialize()
    {
        m_builtinResources.Initialize(this);
    }

    void GFXResourceRegistry::Terminate()
    {
        m_builtinResources.Terminate();
        {
            std::lock_guard<std::mutex> lock(m_layoutCacheMutex);
            m_layoutCache.clear();
        }
        FlushDestroyed();
    }

    GFXDescriptorSetLayoutPtr GFXResourceRegistry::GetOrCreateDescriptorSetLayout(
        std::span<const GFXDescriptorLayoutDesc> layouts)
    {
        std::vector<const GFXDescriptorLayoutDesc*> sorted;
        sorted.reserve(layouts.size());
        for (size_t i = 0; i < layouts.size(); ++i)
            sorted.push_back(&layouts[i]);
        std::sort(sorted.begin(), sorted.end(),
            [](const GFXDescriptorLayoutDesc* a, const GFXDescriptorLayoutDesc* b)
            { return a->BindingPoint < b->BindingPoint; });

        std::string key;
        char buf[64];
        for (const GFXDescriptorLayoutDesc* desc : sorted)
        {
            snprintf(buf, sizeof(buf), "%u:%d:%u;", desc->BindingPoint,
                static_cast<int>(desc->Type), static_cast<uint32_t>(desc->Stage));
            key += buf;
        }

        std::lock_guard<std::mutex> lock(m_layoutCacheMutex);
        auto it = m_layoutCache.find(key);
        if (it != m_layoutCache.end())
            return it->second;

        auto layout = CreateDescriptorSetLayout(layouts);
        m_layoutCache.emplace(std::move(key), layout);
        return layout;
    }

    void GFXResourceRegistry::Tick(uint64_t currentTimeline)
    {
        std::vector<std::unique_ptr<GFXResource>> expired;
        {
            std::lock_guard<std::mutex> lock(m_resourcesMutex);
            auto end = m_deferredDestroys.upper_bound(currentTimeline);
            for (auto it = m_deferredDestroys.begin(); it != end;)
            {
                expired.push_back(std::move(it->second));
                it = m_deferredDestroys.erase(it);
            }
        }
        // Resource destructors run here, outside the lock.
    }

    void GFXResourceRegistry::FlushDestroyed()
    {
        Tick(std::numeric_limits<uint64_t>::max());
    }

    void GFXResourceRegistry::QueueDestroyed(GFXResource* resource)
    {
        if (!resource)
            return;

        std::lock_guard<std::mutex> lock(m_resourcesMutex);
        m_activeResources.erase(resource->GetResourceId());

        // Tag with the submission currently being recorded (framecount+1): the resource
        // may still be referenced by it. Tick() receives the completed timeline and
        // destroys everything whose tag has been passed by the GPU.
        m_deferredDestroys.emplace(
            GetApplication()->GetFrameCount() + 1,
            std::unique_ptr<GFXResource>(resource));
    }

    void GFXResourceRegistry::DestroyResource(GFXResource* resource)
    {
        if (!resource)
            return;

        auto* registry = resource->GetResourceRegistry();
        assert(registry);
        registry->QueueDestroyed(resource);
    }
}
