#pragma once
#include <Pulsar/EngineMath.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <functional>
#include <atomic>
#include <cassert>
#include <memory>

namespace gfx { class GFXResourceManager; }

namespace pulsar
{

    /**
     * @brief 渲染线程主控类。
     *
     * 设计原则：
     * - 所有直接操作 GPU 的函数（vkCreate* / vkCmd* / vkDestroy*）必须在渲染线程执行。
     * - 无后缀的函数默认 asserts 当前是渲染线程。
     * - _AnyThread 后缀的函数可在任意线程调用（通常是主线程），内部通过命令队列异步提交给渲染线程。
     *
     * 示例：
     *   // 主线程调用
     *   TextureHandle h = renderThread->CreateTexture_AnyThread(desc);
     *   renderThread->UploadTexture_AnyThread(h, pixels, size);
     *
     *   // 渲染线程内部调用（如 RenderGraph Execute 时）
     *   TextureEntry* tex = renderThread->GetTexture(h);   // 无后缀，asserts 渲染线程
     */
    class RenderThread
    {
    public:
        RenderThread() = default;
        ~RenderThread() { Stop_AnyThread(); }

        // ========== 启动/停止（_AnyThread）==========
        void Start_AnyThread();
        void Stop_AnyThread();

        // ========== 线程检查 ==========
        bool IsRenderThread() const;

        /**
         * @brief 主线程等待渲染线程完成当前所有工作（用于退出或 Resize）。
         */
        void WaitForIdle_AnyThread();

        // ========== 资源更新队列（_AnyThread）==========
        /**
         * @brief 资源更新操作的回调签名。
         * 主线程构造 lambda 并按值捕获所需数据，渲染线程执行时传入 GFXResourceManager。
         */
        using ResourceUpdateFn = std::move_only_function<void(gfx::GFXResourceManager*)>;

        /**
         * @brief 提交一个资源更新操作到渲染线程的更新队列。
         * 主线程调用，线程安全。
         */
        void EnqueueUpdate_AnyThread(ResourceUpdateFn fn);

        // ========== 帧驱动（Lockstep）==========
        /**
         * @brief 派发一帧渲染到渲染线程并阻塞直到完成（lockstep）。
         * 主线程在游戏 tick 后调用：渲染线程会先抽干资源更新队列（帧屏障，
         * 保证本帧用到的资源已就绪），再执行 renderFn（实际绘制+present）。
         * 由于 game tick 与渲染不重叠，渲染线程可安全直接读取活动场景数据。
         */
        void RunFrame_AnyThread(std::move_only_function<void()> renderFn);

    private:
        void RenderLoop();
        void ProcessCommands();         // 处理 m_commands 队列
        void ProcessResourceUpdates();  // 处理 m_resourceUpdates 队列
        void FlushDestroyedResources(); // vkWaitForFences 后释放 pending destroy

        // 线程
        std::thread m_thread;
        std::thread::id m_renderThreadId;
        std::atomic<bool> m_running{false};

        // 命令队列（主线程写，渲染线程读）
        struct Command {
            std::function<void()> execute;
        };
        std::vector<Command> m_commands;
        std::mutex m_commandMutex;

        // 资源更新队列（主线程写，渲染线程读）
        std::vector<ResourceUpdateFn> m_resourceUpdates;
        std::mutex m_resourceUpdateMutex;

        // 帧驱动（lockstep）：主线程提交一帧并等待渲染线程完成
        std::mutex m_frameMutex;
        std::condition_variable m_frameCv;
        std::move_only_function<void()> m_pendingFrame;
        bool m_frameRequested = false;
        bool m_frameDone = false;


    };

} // namespace pulsar

// 渲染线程断言宏
#ifndef RENDER_THREAD_ASSERT
#define RENDER_THREAD_ASSERT() \
    assert(IsRenderThread() && "This function must be called on the render thread. " \
           "Use the _AnyThread version if calling from the game thread.")
#endif
