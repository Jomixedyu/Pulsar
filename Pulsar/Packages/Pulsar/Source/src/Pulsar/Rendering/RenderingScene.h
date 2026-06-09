#pragma once
#include "gfx/GFXCommandBuffer.h"
#include "RenderObject.h"
#include <Pulsar/EngineMath.h>
#include <vector>
#include <mutex>

namespace pulsar
{
    /**
     * @brief 渲染线程可见的渲染对象 Proxy。
     *
     * PrimitiveRenderingObject 是主线程对象（如 Component）在渲染线程的投影。
     * 它不持有任何指向主线程对象的指针/引用，只保留纯值数据和 MeshBatch 的拷贝。
     *
     * 设计原则：
     * - 所有字段都是值类型或 GPU 资源句柄（gfx::BufferHandle 等），渲染线程可安全读取。
     * - MeshBatch 列表由主线程在提取阶段从 RenderObject::GetMeshBatches() 拷贝而来。
     * - 因此渲染线程无需知道 StaticMesh、SkinnedMesh、InstanceMesh 的区别——差异已封装在 MeshBatch 内。
     *
     * 生命周期：
     * - 主线程在 Component::BeginComponent() 时创建，通过 RenderingScene::Add() 注册。
     * - 主线程每帧 Tick() 后更新 worldMatrix、worldBounds 等动态字段，并重新拷贝 MeshBatch。
     * - 主线程在 Component::EndComponent() 时通过 RenderingScene::Remove() 请求删除。
     * - Proxy 的实际销毁由主线程在 RenderingScene::Flush() 后执行；其引用的 GPU 资源通过 DeferredDestroyQueue 延迟释放。
     *
     * 线程安全：
     * - 主线程写：Tick 期间更新动态字段，Flush 前完成。
     * - 渲染线程读：Flush 之后、Signal 渲染线程之前，主线程不再修改本对象。
     * - 因此渲染线程读取时无需加锁。
     */
    class PrimitiveRenderingObject
    {
    public:
        /** @brief 世界空间变换矩阵。主线程每帧更新。 */
        Matrix4f worldMatrix;

        /** @brief 世界空间轴对齐包围盒。主线程每帧更新。 */
        BoxBounds3f worldBounds;

        /** @brief PerRenderObjectDataManager 中的 slot 索引，用于动态常量缓冲区偏移。 */
        uint32_t perObjectSlot = 0;

        /**
         * @brief 该对象的所有绘制批次。
         *
         * 由主线程从 RenderObject::GetMeshBatches() 拷贝而来。
         * 包含 VertexBuffer、IndexBuffer、Material、PipelineState 等 GPU 资源描述。
         *
         * 注意：渲染队列（Opaque / Transparent / Overlay）由 MeshBatch::Material 或 MeshBatch::Priority 决定，
         * 不在 PrimitiveRenderingObject 上。因此一个 PrimitiveRenderingObject 的不同 MeshBatch 可以属于不同队列。
         *
         * 渲染线程遍历此数组直接发送绘制指令，无需再通过虚函数分发。
         */
        array_list<rendering::MeshBatch> batches;
    };

    /**
     * @brief 渲染线程的场景容器。
     *
     * RenderingScene 持有当前帧所有待渲染的 PrimitiveRenderingObject 指针。
     * 渲染线程遍历该数组，执行视锥剔除（基于 worldBounds），按 MeshBatch 的 Material/Priority/Depth 排序，并直接读取 MeshBatch 发送绘制指令。
     *
     * 线程模型：
     * - 主线程：调用 Add() / Remove()，将变更记录到 pending 队列。
     * - 主线程：在 Tick 之后调用 Flush()，将 pending 变更合并到 m_proxies。
     * - 渲染线程：只读 m_proxies（通过 GetProxies()），不做任何修改。
     */
    class RenderingScene
    {
    public:
        /** @brief 注册一个渲染对象 Proxy。主线程调用，线程安全。 */
        void Add(PrimitiveRenderingObject* object);

        /** @brief 请求移除一个渲染对象 Proxy。主线程调用，线程安全。 */
        void Remove(PrimitiveRenderingObject* object);

        /**
         * @brief 同步 pending 的 Add/Remove 操作到主数组。
         *
         * 必须在主线程 Tick 之后、Signal 渲染线程之前调用。
         * 调用后，m_proxies 即反映最新的场景状态，渲染线程可安全读取。
         */
        void Flush();

        /** @brief 获取当前帧的 Proxy 数组。仅渲染线程调用。 */
        const std::vector<PrimitiveRenderingObject*>& GetProxies() const { return m_proxies; }

    private:
        /** @brief 当前帧渲染线程读取的 Proxy 数组。仅 Flush 修改，渲染线程只读。 */
        std::vector<PrimitiveRenderingObject*> m_proxies;

        /** @brief 待添加的 Proxy。主线程写，Flush 时合并到 m_proxies。 */
        std::vector<PrimitiveRenderingObject*> m_pendingAdd;

        /** @brief 待移除的 Proxy。主线程写，Flush 时从 m_proxies 删除。 */
        std::vector<PrimitiveRenderingObject*> m_pendingRemove;

        /** @brief 保护 pending 队列的锁。Add/Remove/Flush 使用。 */
        std::mutex m_mutex;
    };

} // namespace pulsar
