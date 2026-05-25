#include "RenderThread.h"
#include "RenderProxyRegistry.h"
#include "PerRenderObjectDataManager.h"
#include <Pulsar/CameraManager.h>
#include <Pulsar/Rendering/LightingData.h>

namespace pulsar
{
    RenderThread& RenderThread::Get()
    {
        static RenderThread instance;
        return instance;
    }

    void RenderThread::Start()
    {
        m_running.store(true);
        m_cameraManager = std::make_unique<CameraManager>();
        m_lightManager = std::make_unique<LightManager>();
        m_proxyRegistry = std::make_unique<rendering::RenderProxyRegistry>();
        m_perObjectDataMgr = std::make_unique<PerRenderObjectDataManager>();
    }

    void RenderThread::Stop()
    {
        FlushCommands();
        m_running.store(false);
        m_proxyRegistry.reset();
        m_perObjectDataMgr.reset();
        m_lightManager.reset();
        m_cameraManager.reset();
    }

    void RenderThread::TriggerRenderFrame()
    {
        // Phase 2: process commands immediately.
        // In real thread mode this would swap queues and signal the worker thread.
        ProcessCommands();
        if (m_lightManager)
        {
            m_lightManager->Update();
        }
    }

    void RenderThread::FlushCommands()
    {
        ProcessCommands();
    }

    bool RenderThread::IsInRenderThread() const
    {
        // Phase 2: always true — no separate thread yet.
        return true;
    }

    void RenderThread::Run()
    {
        // Reserved for real thread mode (Phase 3+).
    }

    void RenderThread::ProcessCommands()
    {
        if (!m_hasPendingCommands.load())
            return;

        // Swap queues
        uint32_t readIndex = m_writeQueueIndex.exchange(m_readQueueIndex);
        m_readQueueIndex = readIndex;

        // Execute all commands in the read queue
        auto& queue = m_commandQueue[m_readQueueIndex];
        for (auto& cmd : queue)
        {
            if (cmd)
                cmd();
        }
        queue.clear();

        m_hasPendingCommands.store(false);
    }
}
