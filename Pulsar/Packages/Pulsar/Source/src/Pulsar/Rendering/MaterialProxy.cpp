#include "Rendering/MaterialProxy.h"

#include "Application.h"
#include "Logger.h"

#include <Pulsar/Rendering/ShaderInstanceCache.h>
#include <Pulsar/Rendering/ShaderPropertySync.h>
#include <gfx/GFXResourceManager.h>

#include <utility>

namespace pulsar
{
    MaterialProxy::~MaterialProxy()
    {
        // Runs on whichever thread drops the last shared_ptr; callers route this to the
        // render thread (see Material::DestroyGPUResource) so GPU handles are released there.
        ClearPassBindings();
    }

    const MaterialPassBinding* MaterialProxy::PrepareForRendering(
        const std::string& passName,
        const std::string& interface_)
    {
        // GetPassBinding lazily creates the ShaderInstance for this (pass, interface) if not yet exist
        auto& binding = const_cast<MaterialPassBinding&>(GetPassBinding(passName, interface_));

        auto program = binding.GetCurrentProgram();
        if (!program)
            return nullptr; // shader still compiling

        // A program with no compiled stages (failed builtin/error fallback or empty pending)
        // must never reach pipeline creation — that yields a stageCount==0 VkPipeline.
        if (program->GetGpuPrograms().empty())
            return nullptr;

        // Detect async compilation completing or shader hot-reload for this specific binding
        if (program != binding.m_builtWithProgram.lock())
        {
            // Rebuild GPU resources for this binding with the new program's layout
            binding.m_descriptorSet.reset();
            if (binding.m_descriptorSetLayout.IsValid())
            {
                Application::GetGfxApp()->GetResourceManager()->Destroy(binding.m_descriptorSetLayout);
            }
            binding.m_descriptorSetLayout = gfx::DescriptorSetLayoutHandle{};
            binding.m_materialConstantBuffer.reset();
            binding.m_gpuResourcesInitialized = false;
            EnsureGPUResources(binding, program->m_layout);
            binding.m_builtWithProgram = program;

            // Initial parameter sync: push the current snapshot into freshly created GPU resources.
            ShaderPropertySync::ApplyRenderData(
                m_renderData,
                program->m_layout,
                binding.m_materialConstantBuffer.get(),
                binding.m_descriptorSet.get());
            if (binding.m_descriptorSet)
                binding.m_descriptorSet->Submit();
        }

        if (!binding.m_gpuResourcesInitialized)
            return nullptr;

        return &binding;
    }

    void MaterialProxy::ApplyRenderData(ShaderPropertyRenderData renderData)
    {
        m_renderData = std::move(renderData);

        // Upload the new snapshot to every binding that already has GPU resources ready.
        // Bindings whose shaders haven't compiled yet receive the values via the initial sync
        // inside PrepareForRendering when they eventually become ready.
        for (auto& [key, binding] : m_passBindings)
        {
            if (!binding.m_gpuResourcesInitialized) continue;

            auto program = binding.GetCurrentProgram();
            if (!program) continue;

            ShaderPropertySync::ApplyRenderData(
                m_renderData,
                program->m_layout,
                binding.m_materialConstantBuffer.get(),
                binding.m_descriptorSet.get());

            if (binding.m_descriptorSet)
                binding.m_descriptorSet->Submit();
        }
    }

    void MaterialProxy::UpdateShader(
        SPtr<ShaderConfig> config,
        guid_t shaderGuid,
        std::vector<std::string> features,
        ShaderPassRenderQueueType queue,
        SPtr<ShaderConfigGraphicsPipeline> gpOverride,
        SPtr<ObjectPropertyOverride> gpOverrideFields)
    {
        m_shaderConfig = std::move(config);
        m_shaderGuid = shaderGuid;
        m_activeFeatures = std::move(features);
        m_queue = queue;
        m_graphicsPipelineOverride = std::move(gpOverride);
        m_graphicsPipelineOverrideFields = std::move(gpOverrideFields);

        m_cachedEffectiveGraphicsPipeline.clear();
        ClearPassBindings();
    }

    void MaterialProxy::UpdateStateSnapshot(
        ShaderPassRenderQueueType queue,
        SPtr<ShaderConfigGraphicsPipeline> gpOverride,
        SPtr<ObjectPropertyOverride> gpOverrideFields)
    {
        m_queue = queue;
        m_graphicsPipelineOverride = std::move(gpOverride);
        m_graphicsPipelineOverrideFields = std::move(gpOverrideFields);
        m_cachedEffectiveGraphicsPipeline.clear();
    }

    const MaterialPassBinding& MaterialProxy::GetPassBinding(
        const std::string& passName,
        const std::string& interface_)
    {
        PassKey key{passName, interface_};
        auto it = m_passBindings.find(key);
        if (it != m_passBindings.end())
            return it->second;

        if (!m_shaderConfig)
        {
            static MaterialPassBinding empty{};
            return empty;
        }

        ShaderVariantKey variantKey;
        variantKey.m_shaderGuid = m_shaderGuid;
        variantKey.m_passName = passName;
        variantKey.m_interface = interface_;
        variantKey.m_features = m_activeFeatures;

        ShaderCompileTask task;
        task.m_variantKey = variantKey;

        // Entry point: look it up from the shader config's passes.
        if (m_shaderConfig->Passes)
        {
            // Prefer the matching pass name
            for (const auto& pass : *m_shaderConfig->Passes)
            {
                if (pass->Name == passName && pass->Entry)
                {
                    task.m_entries.m_vertex = pass->Entry->Vertex;
                    task.m_entries.m_fragment = pass->Entry->Fragment;
                    task.m_entries.m_tessControl = pass->Entry->TessControl;
                    task.m_entries.m_tessEval = pass->Entry->TessEval;
                    break;
                }
            }

            // fallback: if no matching pass was found, use the first pass entry
            if (task.m_entries.m_vertex.empty() && task.m_entries.m_fragment.empty())
            {
                for (const auto& pass : *m_shaderConfig->Passes)
                {
                    if (pass->Entry)
                    {
                        task.m_entries.m_vertex = pass->Entry->Vertex;
                        task.m_entries.m_fragment = pass->Entry->Fragment;
                        task.m_entries.m_tessControl = pass->Entry->TessControl;
                        task.m_entries.m_tessEval = pass->Entry->TessEval;
                        Logger::Log("Material pass '" + passName + "' not found, using first pass '" + pass->Name + "' entry as fallback", LogLevel::Warning);
                        break;
                    }
                }
            }
        }

        auto instance = ShaderInstanceCache::Instance().GetOrCreate(variantKey, task);
        MaterialPassBinding binding;
        binding.m_instance = instance;
        auto [insertIt, _] = m_passBindings.emplace(key, std::move(binding));

        return insertIt->second;
    }

    void MaterialProxy::EnsureGPUResources(MaterialPassBinding& binding, const ShaderPropertyLayout& layout)
    {
        if (binding.m_gpuResourcesInitialized)
            return;

        auto gfxApp = Application::GetGfxApp();

        // 创建该 binding 专属的 descriptor set layout (set 0)
        array_list<gfx::GFXDescriptorSetLayoutDesc> descLayoutInfos;

        if (layout.m_totalCBufferSize > 0)
        {
            gfx::GFXDescriptorSetLayoutDesc cbDesc{};
            cbDesc.Type = gfx::GFXDescriptorType::ConstantBuffer;
            cbDesc.Stage = (layout.m_cbufferStageFlags != gfx::GFXGpuProgramStageFlags::None)
                ? layout.m_cbufferStageFlags
                : gfx::GFXGpuProgramStageFlags::VertexFragment;
            cbDesc.BindingPoint = layout.m_cbufferBindingPoint;
            descLayoutInfos.push_back(cbDesc);

            // 创建该 binding 专属的 cbuffer
            gfx::GFXBufferDesc bufferDesc{};
            bufferDesc.Usage = gfx::GFXBufferUsage::ConstantBuffer;
            bufferDesc.StorageType = gfx::GFXBufferMemoryPosition::VisibleOnDevice;
            bufferDesc.BufferSize = layout.m_totalCBufferSize;
            binding.m_materialConstantBuffer = gfxApp->CreateBuffer(bufferDesc);
        }

        for (const auto& texEntry : layout.m_textureEntries)
        {
            gfx::GFXDescriptorSetLayoutDesc texDesc{};
            texDesc.Type = texEntry.m_isCombinedImageSampler
                ? gfx::GFXDescriptorType::CombinedImageSampler
                : gfx::GFXDescriptorType::Texture2D;
            texDesc.Stage = texEntry.m_stageFlags;
            texDesc.BindingPoint = texEntry.m_bindingPoint;
            descLayoutInfos.push_back(texDesc);
        }

        auto* resMgr = Application::GetGfxApp()->GetResourceManager();

        // 即使没有任何 binding 也创建空 layout，确保 set 0 始终存在以保证 set 编号对齐
        binding.m_descriptorSetLayout = resMgr->AllocHandle<gfx::DescriptorSetLayoutHandle>();
        resMgr->CreateDescriptorSetLayout(binding.m_descriptorSetLayout, descLayoutInfos);
        binding.m_descriptorSet = gfxApp->GetDescriptorManager()->GetDescriptorSet(
            resMgr->GetDescriptorSetLayoutShared(binding.m_descriptorSetLayout));

        if (binding.m_materialConstantBuffer)
        {
            binding.m_descriptorSet->AddDescriptor("ConstantProperties", 0)
                ->SetConstantBuffer(binding.m_materialConstantBuffer.get());
        }

        for (const auto& texEntry : layout.m_textureEntries)
        {
            binding.m_descriptorSet->AddDescriptor(texEntry.m_name, texEntry.m_bindingPoint);
        }

        binding.m_gpuResourcesInitialized = true;
    }

    void MaterialProxy::ClearPassBindings()
    {
        if (m_passBindings.empty())
            return;

        // Explicitly destroy handle-managed resources before clearing the map
        auto* resMgr = Application::GetGfxApp()->GetResourceManager();
        for (auto& [key, binding] : m_passBindings)
        {
            if (binding.m_descriptorSetLayout.IsValid())
                resMgr->Destroy(binding.m_descriptorSetLayout);
        }
        m_passBindings.clear();
    }

    bool MaterialProxy::HasPass(const std::string& passName) const
    {
        if (!m_shaderConfig || !m_shaderConfig->Passes)
            return false;
        for (const auto& pass : *m_shaderConfig->Passes)
        {
            if (pass && pass->Name == passName)
                return true;
        }
        return false;
    }

    SPtr<ShaderConfigGraphicsPipeline> MaterialProxy::GetEffectiveGraphicsPipeline(const std::string& passName) const
    {
        auto it = m_cachedEffectiveGraphicsPipeline.find(passName);
        if (it != m_cachedEffectiveGraphicsPipeline.end())
            return it->second;

        SPtr<ShaderConfigGraphicsPipeline> base;
        if (m_shaderConfig && m_shaderConfig->Passes)
        {
            for (const auto& pass : *m_shaderConfig->Passes)
            {
                if (pass->Name == passName && pass->GraphicsPipeline)
                {
                    base = pass->GraphicsPipeline;
                    break;
                }
            }
        }

        if (m_graphicsPipelineOverride && m_graphicsPipelineOverrideFields && !m_graphicsPipelineOverrideFields->IsEmpty())
        {
            auto result = mksptr(new ShaderConfigGraphicsPipeline());
            m_graphicsPipelineOverrideFields->ApplyTo(base.get(), m_graphicsPipelineOverride.get(), result.get());
            m_cachedEffectiveGraphicsPipeline[passName] = result;
            return result;
        }

        m_cachedEffectiveGraphicsPipeline[passName] = base;
        return base;
    }

} // namespace pulsar
