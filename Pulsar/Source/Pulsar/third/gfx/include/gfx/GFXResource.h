#pragma once
#include "GFXResourceType.h"
#include "GFXInclude.h"
#include <atomic>

namespace gfx
{
    class GFXDeferredDestroyQueue;

    // Base class for all GPU resources.
    // Provides intrusive reference counting (AddRef/Release) for thread-safe
    // lifetime management without std::shared_ptr overhead.
    class GFXResource
    {
    public:
        virtual ~GFXResource() = default;
        virtual GFXResourceType GetResourceType() const = 0;

        // Intrusive reference counting
        void AddRef() const
        {
            ++m_refCount;
        }

        void ReleaseRef() const;

        uint32_t GetRefCount() const
        {
            return m_refCount.load();
        }

        // Legacy resource ID (may be removed once slot-based manager is gone)
        uint32_t GetResourceId() const { return m_resourceId; }
        void SetResourceId(uint32_t id) { m_resourceId = id; }

    private:
        mutable std::atomic<uint32_t> m_refCount{0};
        uint32_t m_resourceId = UINT32_MAX;
        mutable std::atomic<bool> m_queuedDestroy{false};
    };
    GFX_DECL_SPTR(GFXResource);

} // namespace gfx
