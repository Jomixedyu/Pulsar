#include "ShaderInstanceCache.h"
#include <Pulsar/Rendering/IShaderCompileService.h>

namespace pulsar
{
    ShaderInstanceCache& ShaderInstanceCache::Instance()
    {
        static ShaderInstanceCache instance;
        return instance;
    }

    void ShaderInstanceCache::Initialize(
        std::shared_ptr<ShaderProgramResource> pendingProgram,
        std::shared_ptr<ShaderProgramResource> errorProgram)
    {
        m_pendingProgram = std::move(pendingProgram);
        m_errorProgram = std::move(errorProgram);
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

        auto instance = std::make_shared<ShaderInstance>(m_pendingProgram);
        m_cache[key] = instance;

        // TODO: 磁盘缓存查询逻辑
        // 如果磁盘缓存命中，加载 SPIR-V 直接创建 Ready 实例

        auto* compileService = ShaderCompileServiceLocator::Get();
        if (compileService)
        {
            ShaderCompileTask task = compileTask;
            std::weak_ptr<ShaderInstance> weakInstance = instance;
            task.m_callback = [weakInstance, errorProgram = m_errorProgram](const ShaderCompileResult& result)
            {
                auto inst = weakInstance.lock();
                if (!inst) return;

                if (result.m_success)
                {
                    inst->ReplaceProgram(result.m_program, ShaderCompileState::Ready);
                }
                else
                {
                    inst->ReplaceProgram(errorProgram, ShaderCompileState::Error);
                }
            };
            instance->ReplaceProgram(m_pendingProgram, ShaderCompileState::Compiling);
            compileService->RequestCompile(task);
        }

        return instance;
    }

    void ShaderInstanceCache::Clear()
    {
        std::lock_guard lock(m_mutex);
        m_cache.clear();
        m_pendingProgram.reset();
        m_errorProgram.reset();
    }

    void ShaderInstanceCache::InvalidateByShader(const guid_t& shaderGuid)
    {
        std::lock_guard lock(m_mutex);

        for (auto& [key, instance] : m_cache)
        {
            if (key.m_shaderGuid == shaderGuid)
            {
                // TODO: 需要重新构造 CompileTask 并提交
            }
        }
    }
}
