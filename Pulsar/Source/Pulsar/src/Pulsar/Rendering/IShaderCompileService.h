#pragma once
#include <Pulsar/Rendering/ShaderProgramResource.h>

#include <functional>
#include <string>
#include <vector>

namespace pulsar
{
    struct ShaderCompileEntries
    {
        std::string m_vertex;
        std::string m_fragment;
        std::string m_tessControl;
        std::string m_tessEval;
    };

    struct ShaderCompileResult
    {
        bool m_success{};
        std::string m_errorMessage;
        std::shared_ptr<ShaderProgramResource> m_program;
    };

    using ShaderCompileCallback = std::function<void(const ShaderCompileResult&)>;

    struct ShaderCompileTask
    {
        ShaderCompileEntries m_entries;
        std::vector<std::string> m_defines;
        ShaderVariantKey m_variantKey;
        ShaderCompileCallback m_callback;
    };

    class IShaderCompileService
    {
    public:
        virtual ~IShaderCompileService() = default;
        // Async: enqueues task, calls task.m_callback on main thread via FlushCallbacks
        virtual void RequestCompile(const ShaderCompileTask& task) = 0;
        // Sync: compiles on the calling thread, returns result immediately
        virtual ShaderCompileResult CompileSync(const ShaderCompileTask& task) = 0;
    };

    class ShaderCompileServiceLocator
    {
    public:
        static void Register(IShaderCompileService* service)
        {
            s_service = service;
        }

        static IShaderCompileService* Get()
        {
            return s_service;
        }

    private:
        static inline IShaderCompileService* s_service = nullptr;
    };
}
