#include "Rendering/MaterialProxy.h"

#include "Application.h"
#include "Logger.h"

#include <Pulsar/Rendering/ShaderInstanceCache.h>
#include <Pulsar/Rendering/ShaderPropertySync.h>
#include <Pulsar/Rendering/DescriptorSetCache.h>
#include <Pulsar/Rendering/DescriptorSetAssembler.h>
#include <Pulsar/Rendering/RenderResourceRegistry.h>
#include <gfx/GFXResourceManager.h>

#include <cassert>
#include <utility>

namespace pulsar
{
    namespace
    {
        // Reflected set0 → the (globally de-duplicated) descriptor set layout for it.
        gfx::GFXDescriptorSetLayout_sp BuildSet0Layout(const ShaderLayout& layout)
        {
            return DescriptorSetAssembler::BuildLayout(layout.FindSet(0));
        }

        const DescriptorBinding* FindMaterialCBuffer(const ShaderLayout& layout)
        {
            const ShaderPropertySetLayout* set0 = layout.FindSet(0);
            return set0 ? set0->FindBinding(kPerMaterialCBufferName) : nullptr;
        }
    }

    MaterialProxy::~MaterialProxy()
    {
        // Runs on whichever thread drops the last shared_ptr; callers route this to the
        // render thread (see Material::DestroyGPUResource) so GPU handles are released there.
        ClearVariants();
    }

    ResolvedVariant MaterialProxy::ResolveRenderVariant(
        const std::string& passName,
        const std::string& interface_)
    {
        auto instance = GetOrCreateInstance(passName, interface_);
        if (!instance)
            return {};

        auto program = instance->GetCurrentProgram();
        // A ShaderInstance always holds a program (Pending/Error builtin fallback at worst);
        // it is never null. A null here means the instance invariant was violated.
        assert(program && "ShaderInstance returned a null program");

        // A program with no compiled stages (failed builtin/error fallback or empty pending)
        // must never reach pipeline creation — that yields a stageCount==0 VkPipeline.
        if (program->GetGpuPrograms().empty())
            return {};

        // The single shared PerMaterial cbuffer is sized from the first ready program that
        // declares a material cbuffer. All variants share one cbuffer (layout forced identical,
        // validated in the shader compile layer).
        EnsurePerMaterialCBuffer(program->m_layout);
        if (m_cbufferDirty)
        {
            UploadCBuffer(program->m_layout);
            m_cbufferDirty = false;
        }

        // Resolve this variant's set0 descriptor set from the global content cache. Build the set0
        // resource registry (shared cbuffer + resolved textures), then Get() a cache-owned set keyed
        // by the (de-duplicated) layout + bound resource identities. Content changes (textures,
        // async-compile swapping the reflected layout) automatically pick a different set — no
        // explicit rebuild/dirty decision, and no "builtin vs compiled" branching.
        ResolvedVariant resolved;
        resolved.m_program = program;

        // set 0 must always be present for set-index alignment, even when the shader declares no
        // set0 bindings — BuildSet0Layout returns a (possibly empty) de-duplicated layout, and the
        // cache hands back a set for it (empty reg -> gfx builtin fallbacks / no writes).
        resolved.m_set0Layout = BuildSet0Layout(program->m_layout);

        const ShaderPropertySetLayout* set0 = program->m_layout.FindSet(0);
        RenderResourceRegistry reg;
        std::vector<gfx::GFXTexture2DView_sp> keepAlive;
        if (set0)
        {
            keepAlive = ShaderPropertySync::BuildSet0Registry(
                m_renderData, *set0, m_perMaterialCBuffer.get(), reg);
        }
        resolved.m_set0 = DescriptorSetCache::Instance().Get(resolved.m_set0Layout, set0, reg);
        return resolved;
    }

    void MaterialProxy::ApplyRenderData(ShaderPropertyRenderData renderData)
    {
        m_renderData = std::move(renderData);
        // Parameter/texture change: mark the cbuffer for re-upload. Set resolution is content-
        // addressed (re-Get each frame), so texture changes need no explicit set rebuild here.
        m_cbufferDirty = true;
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

    std::shared_ptr<ShaderInstance> MaterialProxy::GetOrCreateInstance(
        const std::string& passName,
        const std::string& interface_)
    {
        VariantKey key{passName, interface_};
        auto it = m_variants.find(key);
        if (it != m_variants.end())
            return it->second;

        if (!m_shaderConfig)
            return nullptr;

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
        m_variants.emplace(key, instance);

        return instance;
    }

    void MaterialProxy::EnsurePerMaterialCBuffer(const ShaderLayout& layout)
    {
        if (m_perMaterialCBuffer)
            return;

        const DescriptorBinding* matCbuffer = FindMaterialCBuffer(layout);
        if (!matCbuffer)
            return;

        // 材质唯一 cbuffer（device-local，经 ring staging 队列 transfer 更新）
        gfx::GFXBufferDesc bufferDesc{};
        bufferDesc.Usage = gfx::GFXBufferUsage::ConstantBuffer;
        bufferDesc.StorageType = gfx::GFXBufferMemoryPosition::DeviceLocal;
        bufferDesc.BufferSize = matCbuffer->m_size;
        m_perMaterialCBuffer = Application::GetGfxApp()->CreateBuffer(bufferDesc);
        m_cbufferDirty = true; // seed with the current snapshot on the next resolve
    }

    void MaterialProxy::UploadCBuffer(const ShaderLayout& layout)
    {
        if (!m_perMaterialCBuffer)
            return;
        ShaderPropertySync::UploadPerMaterialCBuffer(m_renderData, layout, m_perMaterialCBuffer.get());
    }

    void MaterialProxy::ClearVariants()
    {
        m_perMaterialCBuffer.reset();
        m_cbufferDirty = false;
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
