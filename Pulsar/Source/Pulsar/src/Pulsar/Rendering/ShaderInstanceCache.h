#pragma once
#include <Pulsar/Rendering/ShaderInstance.h>
#include <Pulsar/Rendering/IShaderCompileService.h>
#include <Pulsar/Assets/Shader.h>

#include <memory>
#include <mutex>
#include <unordered_map>

namespace pulsar
{

    class ShaderInstanceCache
    {
    public:
        static ShaderInstanceCache& Instance();

        // Store builtin shader assets; entry names are resolved at compile time from ShaderConfig
        void Initialize(RCPtr<Shader> pendingShader, RCPtr<Shader> errorShader);

        // Async: returns immediately with Compiling instance, result delivered via FlushCallbacks
        std::shared_ptr<ShaderInstance> GetOrCreate(
            const ShaderVariantKey& key,
            const ShaderCompileTask& compileTask);

        void Clear();

    private:
        ShaderInstanceCache() = default;

        // Build a compile task for a builtin shader, resolving entry names from ShaderConfig by passName+interface
        ShaderCompileTask MakeBuiltinVariantTask(
            const RCPtr<Shader>& shader,
            const ShaderVariantKey& requestedKey) const;

        // Compile and cache a builtin program per variant key (mutex must be held)
        std::shared_ptr<ShaderProgramResource> EnsureBuiltinProgram_Locked(
            const RCPtr<Shader>& shader,
            std::unordered_map<std::string, std::shared_ptr<ShaderProgramResource>>& cache,
            const ShaderVariantKey& requestedKey);

        mutable std::mutex m_mutex;
        std::unordered_map<ShaderVariantKey, std::shared_ptr<ShaderInstance>, ShaderVariantKeyHash> m_cache;

        RCPtr<Shader> m_pendingShader;
        RCPtr<Shader> m_errorShader;

        // Pending/Error programs cached per interface (renderer variant) only — features are irrelevant for builtins
        std::unordered_map<std::string, std::shared_ptr<ShaderProgramResource>> m_pendingByInterface;
        std::unordered_map<std::string, std::shared_ptr<ShaderProgramResource>> m_errorByInterface;
    };
}
