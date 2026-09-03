#include "ShaderInstanceCache.h"
#include <Pulsar/Rendering/IShaderCompileService.h>
#include <Pulsar/Logger.h>

#include <sstream>

namespace pulsar
{
    namespace
    {
        // Canonical string fingerprint of a set0 (PerMaterial) layout. Encodes each binding's
        // bindingPoint/type/size and buffer members (name/offset/size/type), EXCLUDING stage flags
        // (which legitimately differ per pass — set0 may be referenced by different stages).
        std::string FingerprintPerMaterialLayout(const ShaderLayout& layout)
        {
            std::ostringstream os;
            const ShaderPropertySetLayout* set0 = layout.FindSet(0);
            if (!set0)
                return {};

            for (const auto& b : set0->m_bindings)
            {
                os << 'b' << b.m_bindingPoint
                   << ':' << static_cast<int>(b.m_type)
                   << ':' << b.m_size << '{';
                for (const auto& m : b.m_members)
                {
                    os << m.m_name << ',' << m.m_offset << ',' << m.m_size
                       << ',' << static_cast<int>(m.m_type) << ';';
                }
                os << '}';
            }
            return os.str();
        }
    }

    ShaderInstanceCache& ShaderInstanceCache::Instance()
    {
        static ShaderInstanceCache instance;
        return instance;
    }

    void ShaderInstanceCache::Initialize(RCPtr<Shader> pendingShader, RCPtr<Shader> errorShader)
    {
        m_pendingShader = std::move(pendingShader);
        m_errorShader   = std::move(errorShader);
        // programs stay null — compiled lazily on first access
    }

    ShaderCompileTask ShaderInstanceCache::MakeBuiltinVariantTask(
        const RCPtr<Shader>& shader,
        const ShaderVariantKey& requestedKey) const
    {
        ShaderCompileTask task;
        task.m_variantKey = requestedKey;
        task.m_variantKey.m_shaderGuid = shader->GetAssetGuid();

        // Resolve entry names from ShaderConfig by passName. Builtin shaders do not guess a
        // fallback pass: unsupported custom passes stay empty and are skipped until the real
        // shader finishes compilation.
        auto config = shader->GetConfig();
        if (config && config->Passes)
        {
            auto findEntry = [&](const std::string& passName) -> SPtr<ShaderConfigEntry>
            {
                // 先精确匹配 passName
                for (const auto& pass : *config->Passes)
                    if (pass && pass->Entry && pass->Name == passName)
                        return pass->Entry;
                return nullptr;
            };

            if (auto entry = findEntry(requestedKey.m_passName))
            {
                task.m_entries.m_vertex      = entry->Vertex;
                task.m_entries.m_fragment    = entry->Fragment;
                task.m_entries.m_tessControl = entry->TessControl;
                task.m_entries.m_tessEval    = entry->TessEval;
            }
        }

        return task;
    }

    std::shared_ptr<ShaderProgramResource> ShaderInstanceCache::EnsureBuiltinProgram_Locked(
        const RCPtr<Shader>& shader,
        std::unordered_map<std::string, std::shared_ptr<ShaderProgramResource>>& cache,
        const ShaderVariantKey& requestedKey)
    {
        std::string cacheKey = requestedKey.m_passName;
        for (const auto& feature : requestedKey.m_features)
            cacheKey += "|" + feature;

        auto it = cache.find(cacheKey);
        if (it != cache.end())
            return it->second;

        auto task = MakeBuiltinVariantTask(shader, requestedKey);
        auto* svc = ShaderCompileServiceLocator::Get();
        std::shared_ptr<ShaderProgramResource> program;
        if (task.m_entries.m_vertex.empty() && task.m_entries.m_fragment.empty())
        {
            program = std::make_shared<ShaderProgramResource>();
            cache[cacheKey] = program;
            return program;
        }

        if (svc)
        {
            auto result = svc->CompileSync(task);
            program = result.m_success ? result.m_program : std::make_shared<ShaderProgramResource>();
        }
        else
        {
            program = std::make_shared<ShaderProgramResource>();
        }

        cache[cacheKey] = program;
        return program;
    }

    std::shared_ptr<ShaderInstance> ShaderInstanceCache::GetOrCreate(
        const ShaderVariantKey& key,
        const ShaderCompileTask& compileTask)
    {
        std::lock_guard lock(m_mutex);

        auto it = m_cache.find(key);
        if (it != m_cache.end())
        {
            return it->second;
        }

        // Compile builtin programs for this specific pass+features variant
        auto pendingProgram = EnsureBuiltinProgram_Locked(m_pendingShader, m_pendingByVariant, key);
        auto errorProgram   = EnsureBuiltinProgram_Locked(m_errorShader,   m_errorByVariant,   key);

        auto instance = std::make_shared<ShaderInstance>(pendingProgram);
        m_cache[key] = instance;

        auto* compileService = ShaderCompileServiceLocator::Get();
        if (compileService)
        {
            ShaderCompileTask task = compileTask;
            std::weak_ptr<ShaderInstance> weakInstance = instance;
            const guid_t shaderGuid = key.m_shaderGuid;
            task.m_callback = [this, weakInstance, errorProgram, shaderGuid](const ShaderCompileResult& result)
            {
                auto inst = weakInstance.lock();
                if (!inst) return;

                if (result.m_success && result.m_program)
                {
                    // Enforce identical PerMaterial (set0) layout across all variants of this shader.
                    if (!ValidateOrRegisterPerMaterialLayout(shaderGuid, *result.m_program))
                    {
                        Logger::Log(
                            "Shader variant PerMaterial (set0) layout diverges from the shader's "
                            "canonical layout; rejecting variant. All passes/variants of one material "
                            "must share an identical set0 layout.",
                            LogLevel::Error);
                        inst->ReplaceProgram(errorProgram, ShaderCompileState::Error);
                        return;
                    }
                    inst->ReplaceProgram(result.m_program, ShaderCompileState::Ready);
                }
                else
                    inst->ReplaceProgram(errorProgram, ShaderCompileState::Error);
            };
            instance->ReplaceProgram(pendingProgram, ShaderCompileState::Compiling);
            compileService->RequestCompile(task);
        }

        return instance;
    }

    bool ShaderInstanceCache::ValidateOrRegisterPerMaterialLayout(
        const guid_t& shaderGuid,
        const ShaderProgramResource& program)
    {
        const std::string fingerprint = FingerprintPerMaterialLayout(program.m_layout);

        std::lock_guard lock(m_mutex);
        auto it = m_canonicalPerMaterial.find(shaderGuid);
        if (it == m_canonicalPerMaterial.end())
        {
            m_canonicalPerMaterial.emplace(shaderGuid, fingerprint);
            return true;
        }
        return it->second == fingerprint;
    }

    void ShaderInstanceCache::Clear()
    {
        std::lock_guard lock(m_mutex);
        m_cache.clear();
        m_canonicalPerMaterial.clear();
        m_pendingByVariant.clear();
        m_errorByVariant.clear();
    }

    void ShaderInstanceCache::InvalidateShader(const guid_t& shaderGuid)
    {
        std::lock_guard lock(m_mutex);
        for (auto it = m_cache.begin(); it != m_cache.end(); )
        {
            if (it->first.m_shaderGuid == shaderGuid)
                it = m_cache.erase(it);
            else
                ++it;
        }
        m_canonicalPerMaterial.erase(shaderGuid);
    }

}
