#pragma once
#include <CoreLib/Type.h>
#include <functional>
#include <future>
#include <mutex>
#include <vector>
#include <atomic>

namespace pulsar
{
    namespace rendering
    {
        class RenderProxyRegistry;
    }

    // Render Thread singleton.
    // Phase 2: "pseudo-thread" mode — commands are executed immediately on the
    // calling thread. This lets us migrate all call sites to EnqueueCommand()
    // without changing runtime behaviour. In a later phase we will spawn a
    // real background thread and flip IsInRenderThread() to thread-local check.
    class RenderThread
    {
    public:
        static RenderThread& Get();

        // Start / Stop the render thread.
        // Phase 2: Start() does NOT create a new thread.
        void Start();
        void Stop();

        // Enqueue a command to be executed on the Render Thread.
        // Phase 2: executes immediately (synchronously).
        template<typename Func>
        void EnqueueCommand(Func&& func)
        {
            if (IsInRenderThread())
            {
                func();
                return;
            }

            uint32_t writeIndex = m_writeQueueIndex.load();
            std::lock_guard<std::mutex> lock(m_queueMutex);
            m_commandQueue[writeIndex].emplace_back(std::forward<Func>(func));
            m_hasPendingCommands.store(true);
        }

        // Synchronous execute: enqueue and block until finished.
        // If already on Render Thread, executes directly.
        template<typename Func>
        void EnqueueCommandSync(Func&& func)
        {
            if (IsInRenderThread())
            {
                func();
                return;
            }

            std::promise<void> promise;
            auto future = promise.get_future();

            EnqueueCommand([&promise, func = std::forward<Func>(func)]() mutable
            {
                func();
                promise.set_value();
            });

            future.wait();
        }

        // Called once per frame (by Game Thread) to swap queues and process commands.
        // Phase 2: processes commands immediately because there is no separate thread.
        void TriggerRenderFrame();

        // Flush all pending commands (blocks until empty).
        void FlushCommands();

        // Phase 2: always returns true because there is no separate thread.
        bool IsInRenderThread() const;

        rendering::RenderProxyRegistry& GetProxyRegistry() { return *m_proxyRegistry; }

    private:
        void Run();
        void ProcessCommands();

        // Double-buffered command queues
        std::vector<std::function<void()>> m_commandQueue[2];
        std::atomic<uint32_t> m_writeQueueIndex{0};
        uint32_t m_readQueueIndex = 1;

        std::mutex m_queueMutex;
        std::atomic<bool> m_hasPendingCommands{false};
        std::atomic<bool> m_running{false};

        std::unique_ptr<rendering::RenderProxyRegistry> m_proxyRegistry;
    };
}
