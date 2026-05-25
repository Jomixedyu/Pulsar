#include "gfx/GFXResource.h"
#include "gfx/GFXDeferredDestroyQueue.h"

namespace gfx
{
    void GFXResource::ReleaseRef() const
    {
        if (--m_refCount == 0 && !m_queuedDestroy.exchange(true))
        {
            GFXDeferredDestroyQueue::Enqueue(const_cast<GFXResource*>(this));
        }
    }
} // namespace gfx
