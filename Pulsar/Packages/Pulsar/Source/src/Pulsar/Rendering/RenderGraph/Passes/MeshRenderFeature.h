#pragma once
#include "RenderFeature.h"
#include <Pulsar/Rendering/RenderObject.h>
#include <Pulsar/Rendering/ShaderPass.h>
#include <Pulsar/Assets/Material.h>
#include <gfx/GFXDescriptorSet.h>
#include <gfx/GFXCommandBuffer.h>
#include <gfx/GFXApplication.h>
#include <gfx/GFXGraphicsPipelineManager.h>

namespace pulsar
{
    class PerPassResources;

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

        void Initialize(PerPassResources* perPass);
        void Destroy();

    protected:
        virtual std::string GetPerPassLayoutName() const { return "Forward"; }

        gfx::GFXDescriptorSet_sp m_perPassSet;

        static void DrawPreparedBatch(
            gfx::GFXCommandBuffer& cmdBuffer,
            const PreparedBatch& pb,
            gfx::GFXDescriptorSet* perPassSet,
            const SPtr<ShaderConfigGraphicsPipeline>& effectiveGP,
            gfx::GFXGraphicsPipelineManager* pipelineMgr,
            const gfx::GFXRenderTargetDesc& rtDesc);

        template<typename TGetEffectiveGP>
        static void DrawPreparedBatchList(
            gfx::GFXCommandBuffer& cmdBuffer,
            const array_list<PreparedBatch>& entries,
            gfx::GFXDescriptorSet* perPassSet,
            gfx::GFXGraphicsPipelineManager* pipelineMgr,
            const gfx::GFXRenderTargetDesc& rtDesc,
            TGetEffectiveGP getEffectiveGP);
    };

    template<typename TGetEffectiveGP>
    inline void MeshRenderFeature::DrawPreparedBatchList(
        gfx::GFXCommandBuffer& cmdBuffer,
        const array_list<PreparedBatch>& entries,
        gfx::GFXDescriptorSet* perPassSet,
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
            DrawPreparedBatch(cmdBuffer, pb, perPassSet, effectiveGP,
                              pipelineMgr, rtDesc);
        }
    }

} // namespace pulsar
