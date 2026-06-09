#include "RenderThread.h"
#include "RenderingScene.h"
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
        // TODO: 实现等待渲染线程完成当前所有工作的逻辑
        // 可以通过额外的 condition_variable 或 fence 实现
    }

    // ========== 资源更新队列（_AnyThread）==========
    void RenderThread::EnqueueUpdate_AnyThread(ResourceUpdateFn fn)
    {
        std::lock_guard<std::mutex> lock(m_resourceUpdateMutex);
        m_resourceUpdates.push_back(std::move(fn));
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
    void RenderThread::ProcessCommands()
    {
        std::vector<Command> localCommands;
        {
            std::lock_guard<std::mutex> lock(m_commandMutex);
            localCommands.swap(m_commands);
        }

        for (auto& cmd : localCommands)
        {
            if (cmd.execute)
                cmd.execute();
        }
    }

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

            // 1. 先抽干资源更新队列（创建/上传/销毁），形成帧屏障：
            //    保证本帧渲染用到的资源已就绪。
            ProcessResourceUpdates();
            ProcessCommands();

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

        Logger::Log("Render thread stopped");
    }

    void RenderThread::FlushDestroyedResources()
    {
        RENDER_THREAD_ASSERT();
        // TODO: 执行 pending 的 vkDestroy*
    }

} // namespace pulsar
