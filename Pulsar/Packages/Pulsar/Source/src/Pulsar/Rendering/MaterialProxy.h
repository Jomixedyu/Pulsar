#pragma once

#include <gfx/GFXDescriptorSet.h>
#include <gfx/GFXHandle.h>
#include <Pulsar/Rendering/ShaderInstance.h>
#include <Pulsar/Rendering/ShaderPropertyLayout.h>
#include <Pulsar/Rendering/ShaderPropertyRenderData.h>
#include <Pulsar/Rendering/ShaderConfig.h>
#include <Pulsar/Rendering/ObjectPropertyOverride.h>

#include <CoreLib/Guid.h>

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace pulsar
{
    // Per-pass binding key (pass name + renderer interface variant).
    struct PassKey
    {
        std::string m_passName;
        std::string m_interface;

        bool operator==(const PassKey& other) const = default;
        bool operator<(const PassKey& other) const
        {
            if (m_passName != other.m_passName) return m_passName < other.m_passName;
            return m_interface < other.m_interface;
        }
    };

    // A material's unique shader variant (guid + pass + interface + features):
    // references its ShaderInstance AND owns the GPU resources built for that variant.
    struct MaterialVariant
    {
        std::shared_ptr<ShaderInstance> m_shader;

        // Per-variant GPU resources (lazily created when this variant's shader is ready)
        gfx::GFXDescriptorSet_sp             m_descriptorSet;
        gfx::DescriptorSetLayoutHandle       m_descriptorSetLayout;
        gfx::GFXBuffer_sp                    m_materialConstantBuffer;
        bool                                 m_gpuResourcesInitialized = false;
        std::weak_ptr<ShaderProgramResource> m_builtWithProgram;

        std::shared_ptr<ShaderProgramResource> GetCurrentProgram() const
        {
            return m_shader ? m_shader->GetCurrentProgram() : nullptr;
        }
    };

    // Render-thread-owned mirror of a single Material's render state (1:1 with Material).
    //
    // Holds every per-(pass, interface, feature) variant binding plus its GPU resources
    // (descriptor sets, constant buffers), and a snapshot of the parameters / shader config
    // fed by the game thread through the render queue.
    //
    // This is a plain C++ object (NOT ObjectBase): holding and destructing its shared_ptr on
    // the render thread is safe — it never touches RuntimeObjectManager / the dependency graph.
    // All GPU resource creation / async-compile detection happens on the render thread (same as
    // the legacy Material::PrepareForRendering), so the game thread never touches GPU state.
    class MaterialProxy
    {
    public:
        MaterialProxy() = default;
        ~MaterialProxy();

        MaterialProxy(const MaterialProxy&) = delete;
        MaterialProxy& operator=(const MaterialProxy&) = delete;

        // ===== Render thread: consume API (called by render passes) =====

        // Called once per frame before drawing with this (pass, interface).
        // Detects async shader compilation completing, creates GPU resources, and does the
        // initial parameter sync into freshly created resources.
        // Returns nullptr if the shader for this binding is not yet ready.
        const MaterialVariant* ResolveRenderVariant(const std::string& passName, const std::string& interface_);

        // Lazily creates the variant for (pass, interface) without forcing GPU
        // resource creation. Callers that need the GPU resources ready should use ResolveRenderVariant.
        const MaterialVariant& GetVariant(const std::string& passName, const std::string& interface_);

        ShaderPassRenderQueueType GetQueue() const { return m_queue; }

        SPtr<ShaderConfigGraphicsPipeline> GetEffectiveGraphicsPipeline(const std::string& passName) const;

        const SPtr<ShaderConfig>& GetShaderConfig() const { return m_shaderConfig; }

        bool HasPass(const std::string& passName) const;

        // ===== Render thread: feed API (invoked from the render update queue) =====

        // Replace the parameter snapshot and re-apply it to every ready binding.
        void ApplyRenderData(ShaderPropertyRenderData renderData);

        // Replace the shader-related snapshot (config / guid / features / queue / GP override)
        // and clear all bindings so they get rebuilt against the new shader on next prepare.
        void UpdateShader(
            SPtr<ShaderConfig> config,
            guid_t shaderGuid,
            std::vector<std::string> features,
            ShaderPassRenderQueueType queue,
            SPtr<ShaderConfigGraphicsPipeline> gpOverride,
            SPtr<ObjectPropertyOverride> gpOverrideFields);

        // Update light state (queue / graphics-pipeline override) without invalidating bindings.
        void UpdateStateSnapshot(
            ShaderPassRenderQueueType queue,
            SPtr<ShaderConfigGraphicsPipeline> gpOverride,
            SPtr<ObjectPropertyOverride> gpOverrideFields);

    private:
        void EnsureGPUResources(MaterialVariant& variant, const ShaderLayout& layout);
        void ClearVariants();

    private:
        // Snapshot fed by the game thread (immutable plain data / SPtr to non-ObjectBase data; no RCPtr).
        SPtr<ShaderConfig>                 m_shaderConfig;
        guid_t                             m_shaderGuid;
        std::vector<std::string>           m_activeFeatures;
        ShaderPassRenderQueueType          m_queue = ShaderPassRenderQueueType::Opaque;
        ShaderPropertyRenderData           m_renderData;
        SPtr<ShaderConfigGraphicsPipeline> m_graphicsPipelineOverride;
        SPtr<ObjectPropertyOverride>       m_graphicsPipelineOverrideFields;

        // Render-thread-owned GPU state.
        std::map<PassKey, MaterialVariant> m_variants;
        mutable std::map<std::string, SPtr<ShaderConfigGraphicsPipeline>> m_cachedEffectiveGraphicsPipeline;
    };

} // namespace pulsar
