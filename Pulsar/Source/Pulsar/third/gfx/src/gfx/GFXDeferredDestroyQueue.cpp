#include "gfx/GFXDeferredDestroyQueue.h"
#include "gfx/GFXResource.h"

namespace gfx
{
    std::mutex GFXDeferredDestroyQueue::s_mutex;
    std::array<std::vector<GFXResource*>, GFXDeferredDestroyQueue::kFrameBufferCount> GFXDeferredDestroyQueue::s_frames;
    uint32_t GFXDeferredDestroyQueue::s_currentFrame = 0;

    void GFXDeferredDestroyQueue::Enqueue(GFXResource* resource)
    {
        if (!resource)
            return;
        std::lock_guard<std::mutex> lock(s_mutex);
        s_frames[s_currentFrame % kFrameBufferCount].push_back(resource);
    }

    void GFXDeferredDestroyQueue::AdvanceFrame()
    {
        std::lock_guard<std::mutex> lock(s_mutex);

        // Destroy resources that were enqueued kDeferredDestroyFrames ago
        uint32_t destroyIndex = (s_currentFrame + kDeferredDestroyFrames) % kFrameBufferCount;
        auto frame = std::move(s_frames[destroyIndex]);
        s_frames[destroyIndex].clear();
        for (GFXResource* resource : frame)
        {
            delete resource;
        }

        s_currentFrame++;
    }
}
