#pragma once
#include "RenderFeature.h"
#include <Pulsar/Rendering/RenderObject.h>
#include <Pulsar/Rendering/ShaderPass.h>
#include <Pulsar/Rendering/RenderResourceRegistry.h>
#include <Pulsar/Assets/Material.h>
#include <gfx/GFXDescriptorSet.h>
#include <gfx/GFXCommandBuffer.h>
#include <gfx/GFXApplication.h>
#include <gfx/GFXGraphicsPipelineManager.h>

namespace pulsar
{
    struct PreparedBatch
    {
        rendering::MeshBatch batch;
        // Compiled program driving pipeline creation (null => shader not ready, skip).
        std::shared_ptr<ShaderProgramResource> program;
        // The PerMaterial (set0) descriptor set + layout resolved from the global content cache
        // (via MaterialProxy::ResolveRenderVariant). Bound as set 0 for the draw.
        gfx::GFXDescriptorSet*         set0 = nullptr;
        gfx::GFXDescriptorSetLayout_sp set0Layout;
    };

    class MeshRenderFeature : public RenderFeature
    {
    public:
        ~MeshRenderFeature() override { Destroy(); }

        void Initialize() override;
        void Destroy() override;

    protected:
        virtual std::string GetPerPassLayoutName() const { return "Forward"; }

        // Resolve the per-pass (set1) descriptor set for a draw from the global content cache,
        // using the draw's program reflection (set1 layout may differ per shader) + the shared
        // per-pass registry (camera/world/lights/perObject buffers). Bound as set 1.
        static gfx::GFXDescriptorSet* ResolvePerPassSet(
            const PreparedBatch& pb, const RenderResourceRegistry& reg);

        static void DrawPreparedBatch(
            gfx::GFXCommandBuffer& cmdBuffer,
            const PreparedBatch& pb,
            const RenderResourceRegistry& reg,
            const SPtr<ShaderConfigGraphicsPipeline>& effectiveGP,
            gfx::GFXGraphicsPipelineManager* pipelineMgr,
            const gfx::GFXRenderTargetDesc& rtDesc);

        template<typename TGetEffectiveGP>
        static void DrawPreparedBatchList(
            gfx::GFXCommandBuffer& cmdBuffer,
            const array_list<PreparedBatch>& entries,
            const RenderResourceRegistry& reg,
            gfx::GFXGraphicsPipelineManager* pipelineMgr,
            const gfx::GFXRenderTargetDesc& rtDesc,
            TGetEffectiveGP getEffectiveGP);
    };

    template<typename TGetEffectiveGP>
    inline void MeshRenderFeature::DrawPreparedBatchList(
        gfx::GFXCommandBuffer& cmdBuffer,
        const array_list<PreparedBatch>& entries,
        const RenderResourceRegistry& reg,
        gfx::GFXGraphicsPipelineManager* pipelineMgr,
        const gfx::GFXRenderTargetDesc& rtDesc,
        TGetEffectiveGP getEffectiveGP)
    {
        for (const auto& pb : entries)
        {
            if (!pb.program)
                continue;

            if (!pb.batch.Material || !pb.batch.Material->GetShaderConfig())
                continue;

            auto effectiveGP = getEffectiveGP(pb);
            DrawPreparedBatch(cmdBuffer, pb, reg, effectiveGP,
                              pipelineMgr, rtDesc);
        }
    }

} // namespace pulsar
