#include <Pulsar/Rendering/ShaderProxy.h>

#include <Pulsar/Application.h>
#include <Pulsar/Logger.h>
#include <Pulsar/Rendering/RenderThread.h>

#include <CoreLib.Serialization/JsonSerializer.h>

#include <gfx/GFXResourceRegistry.h>

#include <cassert>
#include <sstream>

namespace pulsar
{
    namespace
    {
        // Fingerprint of the PerMaterial cbuffer's binary layout -- the one
        // resource shared across all variants of a material (sized once, uploaded
        // once). Other set0 bindings (textures) may legitimately differ per
        // variant: each variant gets its own descriptor set from the content cache.
        std::string FingerprintPerMaterialLayout(const ShaderLayout& layout)
        {
            const ShaderPropertySetLayout* set0 = layout.FindSet(0);
            const DescriptorBinding* cbuffer = set0 ? set0->FindBinding(kPerMaterialCBufferName) : nullptr;
            if (!cbuffer)
                return {};

            std::ostringstream os;
            os << cbuffer->m_size << '{';
            for (const auto& m : cbuffer->m_members)
            {
                os << m.m_name << ',' << m.m_offset << ',' << m.m_size
                   << ',' << static_cast<int>(m.m_type) << ';';
            }
            os << '}';
            return os.str();
        }

        // Content comparison for cloned configs (clones are new objects, so
        // identity comparison is meaningless; serialize and compare).
        bool SameShaderConfig(const SPtr<ShaderConfig>& a, const SPtr<ShaderConfig>& b)
        {
            if (a == b)
                return true;
            if (!a || !b)
                return false;
                return ser::JsonSerializer::Serialize(a.get(), {})
                == ser::JsonSerializer::Serialize(b.get(), {});
        }

        // Turns opaque stage code blobs into backend GPU programs. Render thread only.
        std::vector<gfx::GFXGpuProgramPtr> CreateGpuPrograms(
            gfx::GFXResourceRegistry* registry, const std::vector<ShaderStageData>& stages)
        {
            std::vector<gfx::GFXGpuProgramPtr> programs;
            programs.reserve(stages.size());
            for (const auto& stage : stages)
            {
                auto program = registry->CreateGpuProgram(
                    stage.m_stage, stage.m_code.data(), stage.m_code.size());
                program->SetEntryName(stage.m_entryName);
                programs.push_back(program);
            }
            return programs;
        }
    }

    ShaderProxy::ShaderProxy(guid_t guid, SPtr<ShaderConfig> config)
        : m_guid(guid), m_config(std::move(config))
    {
    }

    std::shared_ptr<ShaderProgramInstance> ShaderProxy::GetOrCreateVariant(
        const std::string& passName, const std::vector<std::string>& features)
    {
        assert(Application::GetRenderThread()->IsRenderThread() && "ShaderProxy is render-thread only");

        VariantKey key{ passName, features };
        auto it = m_variants.find(key);
        if (it != m_variants.end())
            return it->second;

        auto& builtin = BuiltinShaderPrograms::Instance();
        auto pendingProgram = builtin.GetBuiltinVariant(false, passName, features);
        auto errorProgram = builtin.GetBuiltinVariant(true, passName, features);

        auto instance = std::make_shared<ShaderProgramInstance>(pendingProgram);
        m_variants.emplace(key, instance);

        auto* compileService = ShaderCompileServiceLocator::Get();
        if (compileService && m_config) // config may be null until the asset data loads; stay pending
        {
            auto task = BuildCompileTask(passName, features, true);
            std::weak_ptr<ShaderProgramInstance> weakInstance = instance;
            std::weak_ptr<ShaderProxy> weakSelf = weak_from_this();
            task.m_callback = [weakInstance, weakSelf, errorProgram](const ShaderCompileResult& result)
            {
                // The callback arrives on the main thread; GPU programs are created
                // and the instance is updated on the render thread (which is also
                // the only reader of the instance).
                auto* rt = Application::GetRenderThread();
                if (!rt)
                    return;
                rt->EnqueueUpdate_AnyThread(
                    [weakInstance, weakSelf, errorProgram, result](gfx::GFXResourceRegistry* registry) mutable
                    {
                        auto inst = weakInstance.lock();
                        auto self = weakSelf.lock();
                        if (!inst || !self)
                            return;

                        if (result.m_success)
                        {
                            auto program = std::make_shared<ShaderProgram>(
                                std::move(result.m_shaderName),
                                std::move(result.m_layout),
                                CreateGpuPrograms(registry, result.m_stages));

                            if (!self->ValidatePerMaterialLayout(*program))
                            {
                                Logger::Log(
                                    "Shader variant PerMaterial cbuffer layout diverges from the shader's "
                                    "canonical layout; rejecting variant. All passes/variants of one material "
                                    "must share an identical PerMaterial cbuffer.",
                                    LogLevel::Error);
                                inst->ReplaceProgram(errorProgram, ShaderCompileState::Error);
                                return;
                            }
                            inst->ReplaceProgram(program, ShaderCompileState::Ready);
                        }
                        else
                            inst->ReplaceProgram(errorProgram, ShaderCompileState::Error);
                    });
            };
            instance->ReplaceProgram(pendingProgram, ShaderCompileState::Compiling);
            compileService->RequestCompile(task);
        }

        return instance;
    }

    std::shared_ptr<ShaderProgramInstance> ShaderProxy::GetOrCreateVariantSync(
        const std::string& passName, const std::vector<std::string>& features)
    {
        assert(Application::GetRenderThread()->IsRenderThread() && "ShaderProxy is render-thread only");

        VariantKey key{ passName, features };
        auto it = m_variants.find(key);
        if (it != m_variants.end())
            return it->second;

        // No deeper fallback here: the sync path exists for programs that must
        // be usable immediately (the builtin Pending/Error shaders are
        // themselves the fallback of the async path).
        auto instance = std::make_shared<ShaderProgramInstance>(nullptr);
        m_variants.emplace(key, instance);

        auto* compileService = ShaderCompileServiceLocator::Get();
        auto task = BuildCompileTask(passName, features, false);
        // Strict pass matching: an unknown pass has no entries; the variant stays
        // not-ready (empty program) instead of drawing with a guessed program.
        if (compileService && m_config && !task.m_entries.m_vertex.empty())
        {
            instance->ReplaceProgram(nullptr, ShaderCompileState::Compiling);
            auto result = compileService->CompileSync(std::move(task));
            if (result.m_success)
            {
                auto program = std::make_shared<ShaderProgram>(
                    std::move(result.m_shaderName),
                    std::move(result.m_layout),
                    CreateGpuPrograms(Application::GetGfxApp()->GetResourceRegistry(), result.m_stages));
                if (ValidatePerMaterialLayout(*program))
                    instance->ReplaceProgram(std::move(program), ShaderCompileState::Ready);
                else
                    instance->ReplaceProgram(nullptr, ShaderCompileState::Error);
            }
            else
            {
                instance->ReplaceProgram(nullptr, ShaderCompileState::Error);
            }
        }

        return instance;
    }

    ShaderCompileTask ShaderProxy::BuildCompileTask(
        const std::string& passName, const std::vector<std::string>& features, bool allowPassFallback) const
    {
        // Build the compile task from the config snapshot.
        ShaderCompileTask task;
        task.m_variantKey.m_shaderGuid = m_guid;
        task.m_variantKey.m_passName = passName;
        task.m_variantKey.m_features = features;

        if (m_config && m_config->Passes)
        {
            const ShaderConfigPass* selectedPass = nullptr;

            // Prefer the exact pass name.
            for (const auto& pass : *m_config->Passes)
            {
                if (pass && pass->Name == passName && pass->Entry)
                {
                    selectedPass = pass.get();
                    break;
                }
            }
            // Fallback: no matching pass, use the first pass entry.
            if (!selectedPass && allowPassFallback)
            {
                for (const auto& pass : *m_config->Passes)
                {
                    if (pass && pass->Entry)
                    {
                        selectedPass = pass.get();
                        Logger::Log("Material pass '" + passName + "' not found, using first pass '" + pass->Name + "' entry as fallback", LogLevel::Warning);
                        break;
                    }
                }
            }

            if (selectedPass)
            {
                task.m_entries.m_vertex = selectedPass->Entry->Vertex;
                task.m_entries.m_fragment = selectedPass->Entry->Fragment;
                task.m_entries.m_tessControl = selectedPass->Entry->TessControl;
                task.m_entries.m_tessEval = selectedPass->Entry->TessEval;
            }
        }
        return task;
    }

    void ShaderProxy::UpdateConfig(SPtr<ShaderConfig> config)
    {
        assert(Application::GetRenderThread()->IsRenderThread() && "ShaderProxy is render-thread only");
        // Content-identical updates must not clear variants: clearing forces every
        // consumer through an async recompile (Pending flash).
        if (SameShaderConfig(m_config, config))
            return;
        m_config = std::move(config);
        m_variants.clear();
        m_canonicalPerMaterialCBuffer.clear();
    }

    void ShaderProxy::Reload()
    {
        assert(Application::GetRenderThread()->IsRenderThread() && "ShaderProxy is render-thread only");
        m_variants.clear();
        m_canonicalPerMaterialCBuffer.clear();
    }

    bool ShaderProxy::ValidatePerMaterialLayout(const ShaderProgram& program)
    {
        const std::string fingerprint = FingerprintPerMaterialLayout(program.m_layout);
        if (fingerprint.empty())
            return true; // variant declares no PerMaterial cbuffer -- no shared contract
        if (m_canonicalPerMaterialCBuffer.empty())
        {
            m_canonicalPerMaterialCBuffer = fingerprint;
            return true;
        }
        return m_canonicalPerMaterialCBuffer == fingerprint;
    }

    BuiltinShaderPrograms& BuiltinShaderPrograms::Instance()
    {
        static BuiltinShaderPrograms s_instance;
        return s_instance;
    }

    void BuiltinShaderPrograms::Initialize_AnyThread(RCPtr<Shader> pendingShader, RCPtr<Shader> errorShader)
    {
        // Builtin shaders must exist and have a config; a null here means the
        // asset system is broken. The proxies hold cloned config snapshots and
        // own their variants, exactly like regular shaders.
        assert(pendingShader && pendingShader->GetConfig());
        assert(errorShader && errorShader->GetConfig());

        pendingShader->EnsureGPUResource();
        errorShader->EnsureGPUResource();

        auto pending = pendingShader->GetRenderProxy();
        auto error = errorShader->GetRenderProxy();
        assert(pending && error);
        std::lock_guard lock(m_mutex);
        m_pendingProxy = std::move(pending);
        m_errorProxy = std::move(error);
    }

    std::shared_ptr<ShaderProgram> BuiltinShaderPrograms::GetBuiltinVariant(
        bool error, const std::string& passName, const std::vector<std::string>& features)
    {
        assert(Application::GetRenderThread()->IsRenderThread() && "BuiltinShaderPrograms::GetBuiltinVariant is render-thread only");

        std::shared_ptr<ShaderProxy> proxy;
        {
            std::lock_guard lock(m_mutex);
            proxy = error ? m_errorProxy : m_pendingProxy;
        }
        if (!proxy)
            return std::make_shared<ShaderProgram>(); // runtime without builtins: empty = not ready

        auto program = proxy->GetOrCreateVariantSync(passName, features)->GetCurrentProgram();
        return program ? program : std::make_shared<ShaderProgram>();
    }

    void BuiltinShaderPrograms::Clear_AnyThread()
    {
        std::lock_guard lock(m_mutex);
        m_pendingProxy.reset();
        m_errorProxy.reset();
    }
}
