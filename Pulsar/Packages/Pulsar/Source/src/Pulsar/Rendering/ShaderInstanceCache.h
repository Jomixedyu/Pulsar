#pragma once
#include <Pulsar/Rendering/ShaderInstance.h>
#include <Pulsar/Rendering/IShaderCompileService.h>
#include <Pulsar/Assets/Shader.h>

#include <map>
#include <memory>
#include <mutex>
#include <string>
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

        // Remove all cached ShaderInstances for a specific shader asset (by GUID).
        // Call this before notifying Materials so they re-request compilation.
        void InvalidateShader(const guid_t& shaderGuid);

    private:
        ShaderInstanceCache() = default;

        // Build a compile task for a builtin shader, resolving entry names from ShaderConfig by passName+features
        ShaderCompileTask MakeBuiltinVariantTask(
            const RCPtr<Shader>& shader,
            const ShaderVariantKey& requestedKey) const;

        // Compile and cache a builtin program per variant key (mutex must be held)
        std::shared_ptr<ShaderProgramResource> EnsureBuiltinProgram_Locked(
            const RCPtr<Shader>& shader,
            std::unordered_map<std::string, std::shared_ptr<ShaderProgramResource>>& cache,
            const ShaderVariantKey& requestedKey);

        // Validate a freshly compiled program's set0 (PerMaterial) layout against the canonical
        // layout established by the first compiled variant of this shader (by guid). Returns true
        // if it matches (or establishes it as the canonical); false if it diverges (=> treat the
        // variant as a compile error). A whole material shares ONE set0 block, so all its variants
        // MUST agree on the set0 layout. Thread-safe (takes its own lock).
        bool ValidateOrRegisterPerMaterialLayout(
            const guid_t& shaderGuid,
            const ShaderProgramResource& program);

        mutable std::mutex m_mutex;
        std::unordered_map<ShaderVariantKey, std::shared_ptr<ShaderInstance>, ShaderVariantKeyHash> m_cache;

        // Canonical PerMaterial (set0) layout fingerprint per shader guid (stage flags ignored).
        std::unordered_map<guid_t, std::string> m_canonicalPerMaterial;

        RCPtr<Shader> m_pendingShader;
        RCPtr<Shader> m_errorShader;

        // Pending/Error programs cached per pass and renderer feature
        std::unordered_map<std::string, std::shared_ptr<ShaderProgramResource>> m_pendingByVariant;
        std::unordered_map<std::string, std::shared_ptr<ShaderProgramResource>> m_errorByVariant;
    };
}
