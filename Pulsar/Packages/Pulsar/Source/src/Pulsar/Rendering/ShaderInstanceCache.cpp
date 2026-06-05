#include "ShaderInstanceCache.h"
#include <Pulsar/Rendering/IShaderCompileService.h>

namespace pulsar
{
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

        // Resolve entry names from ShaderConfig by passName, fallback to first pass
        auto config = shader->GetConfig();
        if (config && config->Passes)
        {
            auto findEntry = [&](const std::string& passName) -> SPtr<ShaderConfigEntry>
            {
                // 先精确匹配 passName
                for (const auto& pass : *config->Passes)
                    if (pass && pass->Entry && pass->Name == passName)
                        return pass->Entry;
                // fallback: 第一个有 Entry 的 pass
                for (const auto& pass : *config->Passes)
                    if (pass && pass->Entry)
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
        // cache key: interface only (features ignored for builtins)
        const std::string& cacheKey = requestedKey.m_interface;

        auto it = cache.find(cacheKey);
        if (it != cache.end())
            return it->second;

        auto task = MakeBuiltinVariantTask(shader, requestedKey);
        auto* svc = ShaderCompileServiceLocator::Get();
        std::shared_ptr<ShaderProgramResource> program;
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

        // Compile builtin programs for this specific interface+features variant
        auto pendingProgram = EnsureBuiltinProgram_Locked(m_pendingShader, m_pendingByInterface, key);
        auto errorProgram   = EnsureBuiltinProgram_Locked(m_errorShader,   m_errorByInterface,   key);

        auto instance = std::make_shared<ShaderInstance>(pendingProgram);
        m_cache[key] = instance;

        auto* compileService = ShaderCompileServiceLocator::Get();
        if (compileService)
        {
            ShaderCompileTask task = compileTask;
            std::weak_ptr<ShaderInstance> weakInstance = instance;
            task.m_callback = [weakInstance, errorProgram](const ShaderCompileResult& result)
            {
                auto inst = weakInstance.lock();
                if (!inst) return;

                if (result.m_success)
                    inst->ReplaceProgram(result.m_program, ShaderCompileState::Ready);
                else
                    inst->ReplaceProgram(errorProgram, ShaderCompileState::Error);
            };
            instance->ReplaceProgram(pendingProgram, ShaderCompileState::Compiling);
            compileService->RequestCompile(task);
        }

        return instance;
    }

    void ShaderInstanceCache::Clear()
    {
        std::lock_guard lock(m_mutex);
        m_cache.clear();
        m_pendingByInterface.clear();
        m_errorByInterface.clear();
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
    }

}
