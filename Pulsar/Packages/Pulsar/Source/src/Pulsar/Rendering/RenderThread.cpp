#include "RenderThread.h"
#include <Pulsar/Logger.h>
#include <Pulsar/Application.h>
#include <Pulsar/Rendering/DescriptorSetCache.h>
#include <gfx/GFXResourceRegistry.h>
#include <gfx/GFXApplication.h>

namespace pulsar
{
    // ========== 线程检查 ==========
    bool RenderThread::IsRenderThread() const
    {
        return std::this_thread::get_id() == m_renderThreadId;
    }

    // ========== 启动/停止 ==========
    void RenderThread::Start_AnyThread()
    {
        assert(!m_running && "RenderThread already started");
        m_running = true;
        m_thread = std::thread(&RenderThread::RenderLoop, this);
    }

    void RenderThread::Stop_AnyThread()
    {
        if (!m_running) return;

        {
            std::lock_guard<std::mutex> lock(m_frameMutex);
            m_running = false;
        }
        m_frameCv.notify_all();

        if (m_thread.joinable())
            m_thread.join();
    }

    void RenderThread::DrainQueue_AnyThread()
    {
        if (!m_running) return;
        // 派发一个不绘制的帧：渲染线程依次 drain update 队列、destroy 队列；
        // 返回时所有入队操作均已执行完毕。GPU 资源销毁由时间轴门控负责，无需等 GPU 空闲。
        RunFrame_AnyThread(nullptr);
    }

    // ========== 资源更新队列（_AnyThread）==========
    void RenderThread::EnqueueUpdate_AnyThread(ResourceUpdateFn fn)
    {
        std::lock_guard<std::mutex> lock(m_resourceUpdateMutex);
        m_resourceUpdates.push_back(std::move(fn));
    }

    void RenderThread::EnqueueDestroy_AnyThread(ResourceUpdateFn fn)
    {
        std::lock_guard<std::mutex> lock(m_destroyMutex);
        m_destroys.push_back(std::move(fn));
    }

    // ========== 帧驱动（Lockstep）==========
    void RenderThread::Dispatch_AnyThread(std::move_only_function<void()> renderFn)
    {
        if (!m_running) return;

        std::unique_lock<std::mutex> lock(m_frameMutex);
        m_pendingFrame = std::move(renderFn);
        m_frameRequested = true;
        m_frameDone = false;
        m_frameInFlight = true;
        m_frameCv.notify_all();
    }

    void RenderThread::WaitFrame_AnyThread()
    {
        if (!m_running) return;

        std::unique_lock<std::mutex> lock(m_frameMutex);
        if (!m_frameInFlight) return; // nothing dispatched since last wait
        m_frameCv.wait(lock, [this] { return m_frameDone || !m_running; });
        m_frameInFlight = false;
    }

    void RenderThread::RunFrame_AnyThread(std::move_only_function<void()> renderFn)
    {
        Dispatch_AnyThread(std::move(renderFn));
        WaitFrame_AnyThread();
    }

    // ========== 命令队列处理 ==========
    void RenderThread::ProcessResourceUpdates()
    {
        std::vector<ResourceUpdateFn> localUpdates;
        {
            std::lock_guard<std::mutex> lock(m_resourceUpdateMutex);
            localUpdates.swap(m_resourceUpdates);
        }

        auto* registry = Application::GetGfxApp()->GetResourceRegistry();
        for (auto& update : localUpdates)
        {
            if (update)
                update(registry);
        }
    }

    void RenderThread::ProcessDestroys()
    {
        std::vector<ResourceUpdateFn> localDestroys;
        {
            std::lock_guard<std::mutex> lock(m_destroyMutex);
            localDestroys.swap(m_destroys);
        }

        auto* registry = Application::GetGfxApp()->GetResourceRegistry();
        for (auto& destroy : localDestroys)
        {
            if (destroy)
                destroy(registry);
        }
    }

    // ========== 渲染线程主循环 ==========
    void RenderThread::RenderLoop()
    {
        m_renderThreadId = std::this_thread::get_id();
        Logger::Log("Render thread started");

        while (m_running)
        {
            // 等待主线程派发一帧（lockstep）
            std::move_only_function<void()> frame;
            {
                std::unique_lock<std::mutex> lock(m_frameMutex);
                m_frameCv.wait(lock, [this] { return m_frameRequested || !m_running; });
                if (!m_running) break;
                frame = std::move(m_pendingFrame);
                m_pendingFrame = nullptr;
                m_frameRequested = false;
            }

            // 1. 先抽干资源更新队列（创建/上传），形成帧屏障：
            //    保证本帧渲染用到的资源已就绪。
            ProcessResourceUpdates();

            // 1.1 回收最后引用提交已在 GPU 时间轴完成的 descriptor set 缓存项。
            DescriptorSetCache::Instance().Tick();


            // 1.6 处理独立销毁队列（晚于本帧所有 add/update）。
            ProcessDestroys();

            // 2. 执行本帧渲染（acquire / RenderGraph / present）。
            if (frame)
                frame();

            // 3. 通知主线程本帧完成。
            {
                std::lock_guard<std::mutex> lock(m_frameMutex);
                m_frameDone = true;
            }
            m_frameCv.notify_all();
        }

        // 退出前，调用方（World::OnWorldEnd / Application）已先调用 DrainQueue_AnyThread
        // 派发空帧，把入队的 proxy 移除 / RenderScene 销毁全部执行完（GFX device 仍存活）。
        // 因此停止时队列通常已空；万一仍有残留命令，随 ~RenderThread 一起析构
        // （捕获的对象在此销毁，GPU 资源由 gfxApp->Terminate 兜底回收）。

        Logger::Log("Render thread stopped");
    }

} // namespace pulsar
