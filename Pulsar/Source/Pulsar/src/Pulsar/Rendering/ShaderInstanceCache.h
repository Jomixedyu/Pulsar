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

        void Initialize(
            std::shared_ptr<ShaderProgramResource> pendingProgram,
            std::shared_ptr<ShaderProgramResource> errorProgram);

        std::shared_ptr<ShaderProgramResource> GetPendingProgram() const { return m_pendingProgram; }
        std::shared_ptr<ShaderProgramResource> GetErrorProgram() const { return m_errorProgram; }

        std::shared_ptr<ShaderInstance> GetOrCreate(
            const ShaderVariantKey& key,
            const ShaderCompileTask& compileTask);

        void Clear();
        void InvalidateByShader(const guid_t& shaderGuid);

    private:
        ShaderInstanceCache() = default;

        mutable std::mutex m_mutex;
        std::unordered_map<ShaderVariantKey, std::shared_ptr<ShaderInstance>, ShaderVariantKeyHash> m_cache;

        std::shared_ptr<ShaderProgramResource> m_pendingProgram;
        std::shared_ptr<ShaderProgramResource> m_errorProgram;
    };
}
