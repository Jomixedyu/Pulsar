#include <gfx/GFXResource.h>

#include <atomic>

namespace gfx
{
    GFXResource::GFXResource(GFXResourceRegistry* registry)
        : m_registry(registry)
    {
        static std::atomic<ResourceId> nextResourceId{ 1 };
        m_resourceId = nextResourceId.fetch_add(1, std::memory_order_relaxed);

    }

}
