#pragma once
#include <Pulsar/Rendering/ShaderInstance.h>
#include <Pulsar/Rendering/IShaderCompileService.h>

#include <memory>
#include <mutex>
#include <unordered_map>

namespace pulsar
{
    class Shader;

    class ShaderInstanceCache
    {
    public:
        static ShaderInstanceCache& Instance();

        // Store compile tasks for builtin shaders; programs are compiled lazily on first access
        void Initialize(ShaderCompileTask pendingTask, ShaderCompileTask errorTask);

        // Async: returns immediately with Compiling instance, result delivered via FlushCallbacks
        std::shared_ptr<ShaderInstance> GetOrCreate(
            const ShaderVariantKey& key,
            const ShaderCompileTask& compileTask);

        void Clear();

    private:
        ShaderInstanceCache() = default;

        // Build a builtin (pending/error) compile task for a specific interface+features variant
        ShaderCompileTask MakeBuiltinVariantTask(
            const ShaderCompileTask& baseTask,
            const std::string& interface_,
            const std::vector<std::string>& features) const;

        // Compile and cache a builtin program per interface (renderer variant) only — features ignored (mutex must be held)
        std::shared_ptr<ShaderProgramResource> EnsureBuiltinProgram_Locked(
            const ShaderCompileTask& baseTask,
            std::unordered_map<std::string, std::shared_ptr<ShaderProgramResource>>& cache,
            const ShaderVariantKey& requestedKey);

        mutable std::mutex m_mutex;
        std::unordered_map<ShaderVariantKey, std::shared_ptr<ShaderInstance>, ShaderVariantKeyHash> m_cache;

        ShaderCompileTask m_pendingTask; // template task for Pending shader (no variant)
        ShaderCompileTask m_errorTask;   // template task for Error shader (no variant)

        // Pending/Error programs cached per interface (renderer variant) only — features are irrelevant for builtins
        std::unordered_map<std::string, std::shared_ptr<ShaderProgramResource>> m_pendingByInterface;
        std::unordered_map<std::string, std::shared_ptr<ShaderProgramResource>> m_errorByInterface;
    };
}
