#include "ShaderInstanceCache.h"
#include <Pulsar/Rendering/IShaderCompileService.h>

namespace pulsar
{
    ShaderInstanceCache& ShaderInstanceCache::Instance()
    {
        static ShaderInstanceCache instance;
        return instance;
    }

    void ShaderInstanceCache::Initialize(ShaderCompileTask pendingTask, ShaderCompileTask errorTask)
    {
        m_pendingTask = std::move(pendingTask);
        m_errorTask   = std::move(errorTask);
        // programs stay null — compiled lazily on first access
    }

    ShaderCompileTask ShaderInstanceCache::MakeBuiltinVariantTask(
        const ShaderCompileTask& baseTask,
        const std::string& interface_,
        const std::vector<std::string>& features) const
    {
        ShaderCompileTask task  = baseTask;
        task.m_variantKey.m_interface = interface_;
        task.m_variantKey.m_features  = features;
        // pass name is intentionally left empty for builtin shaders
        return task;
    }

    std::shared_ptr<ShaderProgramResource> ShaderInstanceCache::EnsureBuiltinProgram_Locked(
        const ShaderCompileTask& baseTask,
        std::unordered_map<std::string, std::shared_ptr<ShaderProgramResource>>& cache,
        const ShaderVariantKey& requestedKey)
    {
        // Pending/Error shaders only vary by interface (renderer variant); features are ignored
        const std::string& interface_ = requestedKey.m_interface;

        auto it = cache.find(interface_);
        if (it != cache.end())
            return it->second;

        // Compile this interface-variant of the builtin shader synchronously (no features)
        auto task = MakeBuiltinVariantTask(baseTask, interface_, {});
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

        cache[interface_] = program;
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
        auto pendingProgram = EnsureBuiltinProgram_Locked(m_pendingTask, m_pendingByInterface, key);
        auto errorProgram   = EnsureBuiltinProgram_Locked(m_errorTask,   m_errorByInterface,   key);

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

}
