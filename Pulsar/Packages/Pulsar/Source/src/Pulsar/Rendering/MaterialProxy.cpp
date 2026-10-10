#include "Rendering/MaterialProxy.h"

#include "Application.h"
#include "Logger.h"

#include <Pulsar/Rendering/ShaderProxy.h>
#include <Pulsar/Rendering/ShaderPropertySync.h>
#include <Pulsar/Rendering/DescriptorSetCache.h>
#include <Pulsar/Rendering/DescriptorSetAssembler.h>
#include <Pulsar/Rendering/RenderResourceRegistry.h>

#include <cassert>
#include <algorithm>
#include <utility>
#include <gfx/GFXResourceRegistry.h>

namespace pulsar
{
    namespace
    {
        // Reflected set0 → the (globally de-duplicated) descriptor set layout for it.
        gfx::GFXDescriptorSetLayoutPtr BuildSet0Layout(const ShaderLayout& layout)
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
        const std::string& variantFeature)
    {
        auto instance = GetOrCreateInstance(passName, variantFeature);
        if (!instance)
            return {};

        auto program = instance->GetCurrentProgram();
        // A ShaderProgramInstance always holds a program (Pending/Error builtin fallback at worst);
        // it is never null. A null here means the instance invariant was violated.
        assert(program && "ShaderProgramInstance returned a null program");

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
        std::vector<gfx::GFXTexture2DViewPtr> keepAlive;
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
        std::shared_ptr<ShaderProxy> shaderProxy,
        std::vector<std::string> features,
        ShaderPassRenderQueueType queue,
        SPtr<ShaderConfigGraphicsPipeline> gpOverride,
        SPtr<ObjectPropertyOverride> gpOverrideFields)
    {
        m_shaderProxy = std::move(shaderProxy);
        m_shaderGuid = m_shaderProxy ? m_shaderProxy->GetGuid() : guid_t{};
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

    std::shared_ptr<ShaderProgramInstance> MaterialProxy::GetOrCreateInstance(
        const std::string& passName,
        const std::string& variantFeature)
    {
        if (!m_shaderProxy)
            return nullptr;

        // Merge the renderer-feature variant into the material's active features.
        std::vector<std::string> features = m_activeFeatures;
        if (!variantFeature.empty()
            && std::find(features.begin(), features.end(), variantFeature) == features.end())
        {
            features.push_back(variantFeature);
        }
        return m_shaderProxy->GetOrCreateVariant(passName, std::move(features));
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
        m_perMaterialCBuffer = Application::GetGfxApp()->GetResourceRegistry()->CreateBuffer(bufferDesc);
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
    }

    bool MaterialProxy::HasPass(const std::string& passName) const
    {
        auto config = GetShaderConfig();
        if (!config || !config->Passes)
            return false;
        for (const auto& pass : *config->Passes)
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
        auto config = GetShaderConfig();
        if (config && config->Passes)
        {
            for (const auto& pass : *config->Passes)
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
