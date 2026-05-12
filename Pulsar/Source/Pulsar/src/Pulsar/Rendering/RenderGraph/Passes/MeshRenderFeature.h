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
        const MaterialPassBinding* binding = nullptr;
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
            if (!pb.binding)
                continue;

            auto shader = pb.batch.Material->GetShader();
            if (!shader || !shader->GetConfig())
                continue;

            auto effectiveGP = getEffectiveGP(pb);
            DrawPreparedBatch(cmdBuffer, pb, perPassSet, effectiveGP,
                              pipelineMgr, rtDesc);
        }
    }

} // namespace pulsar
