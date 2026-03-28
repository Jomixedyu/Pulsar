#pragma once
#include <PulsarEd/Assembly.h>
#include <Pulsar/Rendering/IShaderCompileService.h>

#include <psc/ShaderCompiler.h>
#include <psc/ShaderReflect.h>

#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <filesystem>

namespace pulsared
{
    class EditorShaderCompileService : public pulsar::IShaderCompileService
    {
    public:
        EditorShaderCompileService();
        ~EditorShaderCompileService();

        void RequestCompile(const pulsar::ShaderCompileTask& task) override;

        // 处理已完成的编译结果回调（需要在主线程调用）
        void FlushCallbacks();

        // 设置磁盘缓存目录
        void SetCacheDirectory(const std::filesystem::path& cacheDir);

    private:
        void WorkerThread();
        void ExecuteCompile(const pulsar::ShaderCompileTask& task);

        std::vector<std::string> BuildDefines(const pulsar::ShaderCompileTask& task) const;

        // 反射 → Layout
        static void ExtractLayout(const psc::ReflectedShaderResources& reflected, pulsar::ShaderPropertyLayout& layout);

        // 磁盘缓存
        size_t ComputeCacheHash(const std::string& hlslSource, const std::vector<std::string>& defines) const;
        std::filesystem::path GetCachePath(size_t hash, const std::string& stageSuffix) const;
        bool TryLoadFromDiskCache(size_t hash, const std::string& stageSuffix, std::vector<char>& outSpirv) const;
        void WriteToDiskCache(size_t hash, const std::string& stageSuffix, const std::vector<char>& spirv) const;

    private:
        std::thread m_workerThread;
        std::queue<pulsar::ShaderCompileTask> m_pendingTasks;
        std::mutex m_taskMutex;
        std::condition_variable m_taskCV;
        std::atomic<bool> m_running{true};

        // 已完成的回调，在主线程 FlushCallbacks 中执行
        struct CompletedCallback
        {
            pulsar::ShaderCompileCallback m_callback;
            pulsar::ShaderCompileResult m_result;
        };
        std::vector<CompletedCallback> m_completedCallbacks;
        std::mutex m_callbackMutex;

        // 磁盘缓存目录
        std::filesystem::path m_cacheDir;
    };
}
