#pragma once
#include <array>
#include <mutex>
#include <vector>
#include <cstdint>

namespace gfx
{
    class GFXResource;

    // Deferred destruction queue for GPU resources.
    // Resources are held for kDeferredDestroyFrames (2) before actual deletion,
    // ensuring the GPU is no longer using them.
    //
    // Thread-safe: Enqueue() can be called from any thread.
    // AdvanceFrame() should be called once per frame (from Render Thread).
    class GFXDeferredDestroyQueue
    {
    public:
        static void Enqueue(GFXResource* resource);
        static void AdvanceFrame();

        static constexpr uint32_t kDeferredDestroyFrames = 2;
        static constexpr uint32_t kFrameBufferCount = kDeferredDestroyFrames + 1;

    private:
        static std::mutex s_mutex;
        static std::array<std::vector<GFXResource*>, kFrameBufferCount> s_frames;
        static uint32_t s_currentFrame;
    };
}
