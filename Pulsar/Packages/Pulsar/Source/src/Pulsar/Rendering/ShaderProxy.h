#pragma once
#include <Pulsar/Rendering/IShaderCompileService.h>
#include <Pulsar/Rendering/ShaderConfig.h>
#include <Pulsar/Rendering/ShaderProgramInstance.h>
#include <Pulsar/Rendering/ShaderProgram.h>

#include <Pulsar/Assets/Shader.h>

#include <CoreLib/Guid.h>

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace pulsar
{
    // Render-thread mirror of one shader asset: holds a config snapshot and every
    // compiled (pass, features) variant of the shader. The config may be null
    // until the asset data loads; variants stay pending and compile once it
    // arrives via UpdateConfig. Owned by the Shader asset
    // (IGPUResource lifecycle: created by the first user, destroyed on unload);
    // consumers receive the pointer through the render queue and never touch the
    // asset. Variants are queried per frame and never cached by consumers, so
    // config updates and hot reloads heal by themselves.
    class ShaderProxy : public std::enable_shared_from_this<ShaderProxy>
    {
    public:
        ShaderProxy(guid_t guid, SPtr<ShaderConfig> config);

        const guid_t& GetGuid() const { return m_guid; }
        const SPtr<ShaderConfig>& GetConfig() const { return m_config; }

        // Render thread only. Returns the current instance for (pass, features),
        // starting async compilation on first request.
        std::shared_ptr<ShaderProgramInstance> GetOrCreateVariant(
            const std::string& passName, const std::vector<std::string>& features);

        // Render thread only. Same as GetOrCreateVariant but compiles
        // synchronously on first request, for programs that must be usable
        // immediately (the builtin Pending/Error fallbacks of the async path).
        std::shared_ptr<ShaderProgramInstance> GetOrCreateVariantSync(
            const std::string& passName, const std::vector<std::string>& features);

        // Render thread only. Swap the config snapshot (no-op when the content is
        // identical) and drop all variants, so they recompile on next request.
        void UpdateConfig(SPtr<ShaderConfig> config);

        // Render thread only. Source reload: drop all variants unconditionally.
        void Reload();

    private:
        struct VariantKey
        {
            std::string m_passName;
            std::vector<std::string> m_features;

            bool operator==(const VariantKey& other) const = default;
            bool operator<(const VariantKey& other) const
            {
                if (m_passName != other.m_passName) return m_passName < other.m_passName;
                return m_features < other.m_features;
            }
        };

        // Enforces an identical PerMaterial cbuffer layout across every variant of
        // this shader that declares one; the first variant with a cbuffer
        // establishes the canonical layout.
        bool ValidatePerMaterialLayout(const ShaderProgram& program);

        // Builds the compile task for (pass, features) from the config snapshot.
        // allowPassFallback: fall back to the first pass entry when the pass is
        // missing (materials); builtin fallbacks must not guess -- an unknown
        // pass yields empty entries and the variant stays not-ready.
        ShaderCompileTask BuildCompileTask(const std::string& passName, const std::vector<std::string>& features, bool allowPassFallback) const;

        guid_t m_guid;
        SPtr<ShaderConfig> m_config;
        std::map<VariantKey, std::shared_ptr<ShaderProgramInstance>> m_variants;
        std::string m_canonicalPerMaterialCBuffer;
    };

    // Access point for the builtin Pending/Error fallback programs that every
    // proxy uses while a variant compiles. The fallbacks live in the builtin
    // shaders' own proxies (compiled synchronously on request, since they ARE
    // the fallback); this class only keeps the proxies alive for the session.
    class BuiltinShaderPrograms
    {
    public:
        static BuiltinShaderPrograms& Instance();

        // Any thread (startup): create the builtin shaders' proxies and keep
        // them alive for the session.
        void Initialize_AnyThread(RCPtr<Shader> pendingShader, RCPtr<Shader> errorShader);

        // Render thread only: the shared Pending/Error program for (pass, features),
        // compiled synchronously on first request.
        std::shared_ptr<ShaderProgram> GetBuiltinVariant(
            bool error, const std::string& passName, const std::vector<std::string>& features);

        // Any thread: releases the builtin proxies (shutdown).
        void Clear_AnyThread();

    private:
        std::shared_ptr<ShaderProxy> m_pendingProxy;
        std::shared_ptr<ShaderProxy> m_errorProxy;
        mutable std::mutex m_mutex;
    };
}
