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
     * @brief 渲染线程主控类（lockstep 模型）。
     *
     * 设计原则：
     * - 所有直接操作 GPU 的函数（vkCreate* / vkCmd* / vkDestroy*）必须在渲染线程执行。
     * - _AnyThread 后缀的函数可在任意线程调用（通常是主线程）：资源创建/上传/销毁通过
     *   update / destroy 队列异步提交，渲染线程在每帧开头按序 drain。
     * - 主线程在游戏 tick 后调用 RunFrame_AnyThread 派发一帧并阻塞至完成；game tick 与
     *   渲染不重叠，故渲染线程可安全直接读取活动场景数据。
     *
     * 帧内执行顺序：drain update -> (可选 vkDeviceWaitIdle) -> drain destroy -> renderFn。
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
         * @brief 主线程等待渲染线程完成当前所有工作并等 GPU 空闲（用于退出 / 卸载 World / Resize）。
         *
         * 派发一个不绘制的帧，渲染线程会：
         *   1. drain update 队列；
         *   2. vkDeviceWaitIdle —— 等 GPU 把已提交命令全部执行完；
         *   3. drain destroy 队列 —— 此时无 in-flight 命令引用待销毁资源，安全释放。
         * 调用返回后，所有已入队的销毁均已在 GPU 完成后执行完毕。
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

        /**
         * @brief 提交一个销毁操作到渲染线程的独立销毁队列。
         * 与 update 队列分离：销毁在每帧 update 之后、渲染之前统一处理，
         * 保证销毁顺序晚于本帧所有 add/update（FIFO 内部 + 队列间有序）。
         * 主线程调用，线程安全。
         */
        void EnqueueDestroy_AnyThread(ResourceUpdateFn fn);

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
        void ProcessResourceUpdates();  // 处理 m_resourceUpdates 队列
        void ProcessDestroys();         // 处理 m_destroys 队列（update 之后）

        // 线程
        std::thread m_thread;
        std::thread::id m_renderThreadId;
        std::atomic<bool> m_running{false};

        // 资源更新队列（主线程写，渲染线程读）
        std::vector<ResourceUpdateFn> m_resourceUpdates;
        std::mutex m_resourceUpdateMutex;

        // 独立销毁队列（主线程写，渲染线程读）。每帧 update 之后处理。
        std::vector<ResourceUpdateFn> m_destroys;
        std::mutex m_destroyMutex;

        // 帧驱动（lockstep）：主线程提交一帧并等待渲染线程完成
        std::mutex m_frameMutex;
        std::condition_variable m_frameCv;
        std::move_only_function<void()> m_pendingFrame;
        bool m_frameRequested = false;
        bool m_frameDone = false;

        // WaitForIdle 请求：渲染线程在 ProcessDestroys 前执行 vkDeviceWaitIdle。
        std::atomic<bool> m_waitDeviceIdle{false};


    };

} // namespace pulsar
