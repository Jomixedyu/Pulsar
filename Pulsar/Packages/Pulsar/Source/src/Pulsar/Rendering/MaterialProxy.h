#pragma once

#include <gfx/GFXDescriptorSet.h>
#include <Pulsar/Rendering/ShaderProgramInstance.h>
#include <Pulsar/Rendering/ShaderProxy.h>
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
    // Per-variant binding key: pass name + renderer feature variant (not just the pass —
    // it identifies a concrete compiled variant of this material's shader).
    // What a render pass needs to bind set0 for a draw: the ready shader program plus the
    // set0 descriptor set + its layout (from the global content cache). Returned by
    // MaterialProxy::ResolveRenderVariant. All pointers are owned elsewhere (program by the
    // ShaderProgramInstance, set/layout by the global DescriptorSetCache). A default-constructed
    // (empty program) value means "not ready this frame" — test it with the bool conversion.
    struct ResolvedVariant
    {
        std::shared_ptr<ShaderProgram> m_program;
        gfx::GFXDescriptorSet*                 m_set0 = nullptr;
        gfx::GFXDescriptorSetLayoutPtr         m_set0Layout;

        // True only when the program is ready to draw (has compiled GPU stages). A program is
        // always present (Pending/Error builtin fallback), but an empty-stage program must not
        // reach pipeline creation, so it reads as not-ready here.
        explicit operator bool() const { return m_program && !m_program->GetGpuPrograms().empty(); }
    };

    // Render-thread-owned mirror of a single Material's render state (1:1 with Material).
    //
    // Holds every per-(pass, renderer feature) variant (its set0 descriptor set + layout) plus a SINGLE
    // PerMaterial constant buffer shared across every pass/variant of this material, plus a
    // snapshot of the parameters / shader config fed by the game thread through the render queue.
    //
    // The PerMaterial cbuffer layout is forced identical across all passes/variants of one
    // material: the first successfully compiled variant establishes the canonical set0 layout
    // (registered per shader guid in the shader's ShaderProxy); any later variant whose set0 layout
    // diverges fails compilation. Because the material cbuffer is guaranteed identical, one buffer
    // serves the whole material — parameter changes pack + upload it ONCE, then each variant's
    // descriptor set references it (textures stay per-variant).
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

        // Called once per frame before drawing with this (pass, renderer feature).
        // Ensures the ShaderProgramInstance exists, detects async compilation completing / hot-reload,
        // lazily builds the shared PerMaterial cbuffer, and acquires (from the global content
        // cache) the set0 descriptor set for this variant's set0 layout.
        // Returns { program, set0 set, set0 layout } ready to bind, or a default-constructed
        // (empty) ResolvedVariant if the shader for this binding is not yet ready (still
        // compiling / errored / empty). Test readiness via the ResolvedVariant bool conversion.
        ResolvedVariant ResolveRenderVariant(const std::string& passName, const std::string& variantFeature = {});

        ShaderPassRenderQueueType GetQueue() const { return m_queue; }

        SPtr<ShaderConfigGraphicsPipeline> GetEffectiveGraphicsPipeline(const std::string& passName) const;

        // The shader config snapshot owned by the shader's ShaderProxy; null while
        // the proxy has not been upserted yet.
        SPtr<ShaderConfig> GetShaderConfig() const { return m_shaderProxy ? m_shaderProxy->GetConfig() : nullptr; }

        bool HasPass(const std::string& passName) const;

        // ===== Render thread: feed API (invoked from the render update queue) =====

        // Replace the parameter snapshot and re-apply it to every ready binding.
        void ApplyRenderData(ShaderPropertyRenderData renderData);

        // Replace the shader (its render-thread proxy, shipped from the game
        // thread) plus features / queue / GP override, and clear all bindings so
        // they get rebuilt on next prepare.
        void UpdateShader(
            std::shared_ptr<ShaderProxy> shaderProxy,
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
        // Looks up or creates the ShaderProgramInstance for (pass, renderer feature). Returns null if there is
        // no shader config to compile against.
    std::shared_ptr<ShaderProgramInstance> GetOrCreateInstance(const std::string& passName, const std::string& variantFeature);
        // Ensures the single shared PerMaterial cbuffer exists (sized from the material cbuffer
        // binding in the given set0 layout). No-op if already built.
        void EnsurePerMaterialCBuffer(const ShaderLayout& layout);
        // Re-packs the shared cbuffer from m_renderData (parameter change; descriptor sets store the
        // buffer handle, not its bytes, so no set touch is needed).
        void UploadCBuffer(const ShaderLayout& layout);
        void ClearVariants();

    private:
        // Snapshot fed by the game thread (immutable plain data; no RCPtr).
        // The shader config is NOT stored here; it comes from the shader's ShaderProxy.
        guid_t                             m_shaderGuid;
        std::shared_ptr<ShaderProxy>       m_shaderProxy;
        std::vector<std::string>           m_activeFeatures;
        ShaderPassRenderQueueType          m_queue = ShaderPassRenderQueueType::Opaque;
        ShaderPropertyRenderData           m_renderData;
        SPtr<ShaderConfigGraphicsPipeline> m_graphicsPipelineOverride;
        SPtr<ObjectPropertyOverride>       m_graphicsPipelineOverrideFields;

        // Render-thread-owned state.
        // Per-(pass, renderer feature) shader instances — pure shader, no GPU resources.
        // Variants live in the shader's ShaderProxy; the proxy re-resolves them
        // every frame, so shader edits and hot reloads heal without any explicit
        // invalidation here.

        // Single PerMaterial constant buffer shared across every variant of this material. set0
        // descriptor sets themselves live in the global DescriptorSetCache (content-addressed);
        // the proxy holds no set/handle and re-resolves each frame.
        gfx::GFXBufferPtr m_perMaterialCBuffer;
        bool              m_cbufferDirty = false; // parameter snapshot changed, re-upload next resolve

        mutable std::map<std::string, SPtr<ShaderConfigGraphicsPipeline>> m_cachedEffectiveGraphicsPipeline;
    };

} // namespace pulsar
