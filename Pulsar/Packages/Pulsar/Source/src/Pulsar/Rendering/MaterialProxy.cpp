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
        ClearVariants();
    }

    const MaterialVariant* MaterialProxy::ResolveRenderVariant(
        const std::string& passName,
        const std::string& interface_)
    {
        // GetVariant lazily creates the variant for this (pass, interface) if not yet exist
        auto& variant = const_cast<MaterialVariant&>(GetVariant(passName, interface_));

        auto program = variant.GetCurrentProgram();
        if (!program)
            return nullptr; // shader still compiling

        // A program with no compiled stages (failed builtin/error fallback or empty pending)
        // must never reach pipeline creation — that yields a stageCount==0 VkPipeline.
        if (program->GetGpuPrograms().empty())
            return nullptr;

        // Detect async compilation completing or shader hot-reload for this specific variant
        if (program != variant.m_builtWithProgram.lock())
        {
            // Rebuild GPU resources for this variant with the new program's layout
            variant.m_descriptorSet.reset();
            if (variant.m_descriptorSetLayout.IsValid())
            {
                Application::GetGfxApp()->GetResourceManager()->Destroy(variant.m_descriptorSetLayout);
            }
            variant.m_descriptorSetLayout = gfx::DescriptorSetLayoutHandle{};
            variant.m_materialConstantBuffer.reset();
            variant.m_gpuResourcesInitialized = false;
            EnsureGPUResources(variant, program->m_layout);
            variant.m_builtWithProgram = program;

            // Initial parameter sync: push the current snapshot into freshly created GPU resources.
            ShaderPropertySync::ApplyRenderData(
                m_renderData,
                program->m_layout,
                variant.m_materialConstantBuffer.get(),
                variant.m_descriptorSet.get());
        }

        if (!variant.m_gpuResourcesInitialized)
            return nullptr;

        return &variant;
    }

    void MaterialProxy::ApplyRenderData(ShaderPropertyRenderData renderData)
    {
        m_renderData = std::move(renderData);

        // Upload the new snapshot to every variant that already has GPU resources ready.
        // Variants whose shaders haven't compiled yet receive the values via the initial sync
        // inside ResolveRenderVariant when they eventually become ready.
        for (auto& [key, variant] : m_variants)
        {
            if (!variant.m_gpuResourcesInitialized) continue;

            // 必须用 set 当初据以创建的 program 的 layout，而非 GetCurrentProgram()。
            // 异步重编后 current 可能领先于 set 的 layout，错配会把 descriptor 写到不存在的
            // binding 上（VUID-VkWriteDescriptorSet-dstBinding-10009）。错配的变体由
            // ResolveRenderVariant 重建 set 时再同步。
            auto program = variant.m_builtWithProgram.lock();
            if (!program) continue;

            ShaderPropertySync::ApplyRenderData(
                m_renderData,
                program->m_layout,
                variant.m_materialConstantBuffer.get(),
                variant.m_descriptorSet.get());
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
        ClearVariants();
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

    const MaterialVariant& MaterialProxy::GetVariant(
        const std::string& passName,
        const std::string& interface_)
    {
        PassKey key{passName, interface_};
        auto it = m_variants.find(key);
        if (it != m_variants.end())
            return it->second;

        if (!m_shaderConfig)
        {
            static MaterialVariant empty{};
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
        MaterialVariant variant;
        variant.m_shader = instance;
        auto [insertIt, _] = m_variants.emplace(key, std::move(variant));

        return insertIt->second;
    }

    void MaterialProxy::EnsureGPUResources(MaterialVariant& variant, const ShaderLayout& layout)
    {
        if (variant.m_gpuResourcesInitialized)
            return;

        auto gfxApp = Application::GetGfxApp();

        // 创建该变体专属的 descriptor set layout (set 0)
        array_list<gfx::GFXDescriptorLayoutDesc> descLayoutInfos;

        // Material params live in set0; convention is a single material cbuffer.
        const ShaderPropertySetLayout* set0 = layout.FindSet(0);
        const DescriptorBinding* matCbuffer = nullptr;

        if (set0)
        {
            for (const auto& b : set0->m_bindings)
            {
                gfx::GFXDescriptorLayoutDesc desc{};
                desc.Type = b.m_type;
                desc.Stage = (b.m_stageFlags != gfx::GFXGpuProgramStageFlags::None)
                    ? b.m_stageFlags
                    : gfx::GFXGpuProgramStageFlags::VertexFragment;
                desc.BindingPoint = b.m_bindingPoint;
                descLayoutInfos.push_back(desc);

                // First non-empty buffer drives the material constant buffer allocation.
                if (!matCbuffer && b.IsBuffer() && b.m_size > 0)
                    matCbuffer = &b;
            }
        }

        if (matCbuffer)
        {
            // 创建该变体专属的 cbuffer
            gfx::GFXBufferDesc bufferDesc{};
            bufferDesc.Usage = gfx::GFXBufferUsage::ConstantBuffer;
            bufferDesc.StorageType = gfx::GFXBufferMemoryPosition::VisibleOnDevice;
            bufferDesc.BufferSize = matCbuffer->m_size;
            variant.m_materialConstantBuffer = gfxApp->CreateBuffer(bufferDesc);
        }

        auto* resMgr = Application::GetGfxApp()->GetResourceManager();

        // 即使没有任何 binding 也创建空 layout，确保 set 0 始终存在以保证 set 编号对齐
        variant.m_descriptorSetLayout = resMgr->AllocHandle<gfx::DescriptorSetLayoutHandle>();
        resMgr->CreateDescriptorSetLayout(variant.m_descriptorSetLayout, descLayoutInfos);
        variant.m_descriptorSet = resMgr->GetDescriptorSetLayoutShared(variant.m_descriptorSetLayout)->AllocateSet();

        // descriptor 由首次 ShaderPropertySync::ApplyRenderData 的 assembler 装配（FindByBinding-then-Add）

        variant.m_gpuResourcesInitialized = true;
    }

    void MaterialProxy::ClearVariants()
    {
        if (m_variants.empty())
            return;

        // Explicitly destroy handle-managed resources before clearing the map
        auto* resMgr = Application::GetGfxApp()->GetResourceManager();
        for (auto& [key, variant] : m_variants)
        {
            if (variant.m_descriptorSetLayout.IsValid())
                resMgr->Destroy(variant.m_descriptorSetLayout);
        }
        m_variants.clear();
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
