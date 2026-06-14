#include "RenderThread.h"
#include <Pulsar/Logger.h>
#include <Pulsar/Application.h>
#include <gfx/GFXResourceManager.h>
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

    void RenderThread::WaitForIdle_AnyThread()
    {
        if (!m_running) return;
        // 派发一个不绘制的帧：渲染线程会先 drain update/command 队列，
        // 然后 vkDeviceWaitIdle 等 GPU 执行完已提交命令，最后 drain destroy 队列。
        // 用于退出 / 卸载 World：返回后所有入队销毁均已在 GPU 完成后执行完毕。
        m_waitDeviceIdle.store(true, std::memory_order_relaxed);
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
    void RenderThread::RunFrame_AnyThread(std::move_only_function<void()> renderFn)
    {
        if (!m_running) return;

        std::unique_lock<std::mutex> lock(m_frameMutex);
        m_pendingFrame = std::move(renderFn);
        m_frameRequested = true;
        m_frameDone = false;
        m_frameCv.notify_all();
        m_frameCv.wait(lock, [this] { return m_frameDone || !m_running; });
    }

    // ========== 命令队列处理 ==========
    void RenderThread::ProcessResourceUpdates()
    {
        std::vector<ResourceUpdateFn> localUpdates;
        {
            std::lock_guard<std::mutex> lock(m_resourceUpdateMutex);
            localUpdates.swap(m_resourceUpdates);
        }

        auto* resMgr = Application::GetGfxApp()->GetResourceManager();
        for (auto& update : localUpdates)
        {
            if (update)
                update(resMgr);
        }
    }

    void RenderThread::ProcessDestroys()
    {
        std::vector<ResourceUpdateFn> localDestroys;
        {
            std::lock_guard<std::mutex> lock(m_destroyMutex);
            localDestroys.swap(m_destroys);
        }

        auto* resMgr = Application::GetGfxApp()->GetResourceManager();
        for (auto& destroy : localDestroys)
        {
            if (destroy)
                destroy(resMgr);
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

            // 1.5 若收到 WaitForIdle 请求，先等 GPU 把已提交命令全部执行完，
            //     再处理销毁队列，避免释放仍被 in-flight 命令引用的资源。
            if (m_waitDeviceIdle.exchange(false, std::memory_order_relaxed))
                Application::GetGfxApp()->WaitDeviceIdle();

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

        // 退出前，调用方（World::OnWorldEnd / Application）已先调用 WaitForIdle_AnyThread
        // 派发空帧，把入队的 proxy 移除 / RenderScene 销毁全部执行完（GFX device 仍存活）。
        // 因此停止时队列通常已空；万一仍有残留命令，随 ~RenderThread 一起析构
        // （捕获的对象在此销毁，GPU 资源由 gfxApp->Terminate 兜底回收）。

        Logger::Log("Render thread stopped");
    }

} // namespace pulsar
