#include "MeshRenderFeature.h"
#include <Pulsar/Rendering/DescriptorSetCache.h>
#include <Pulsar/Rendering/DescriptorSetAssembler.h>
#include <Pulsar/Application.h>

namespace pulsar
{
    void MeshRenderFeature::Initialize()
    {
    }

    void MeshRenderFeature::Destroy()
    {
    }

    gfx::GFXDescriptorSet* MeshRenderFeature::ResolvePerPassSet(
        const PreparedBatch& pb, const RenderResourceRegistry& reg)
    {
        const ShaderPropertySetLayout* set1 = pb.program->m_layout.FindSet(1);
        auto layout = DescriptorSetAssembler::BuildLayout(set1);
        return DescriptorSetCache::Instance().Get(layout, set1, reg);
    }

    void MeshRenderFeature::DrawPreparedBatch(
        gfx::GFXCommandBuffer& cmdBuffer,
        const PreparedBatch& pb,
        const RenderResourceRegistry& reg,
        const SPtr<ShaderConfigGraphicsPipeline>& effectiveGP,
        gfx::GFXGraphicsPipelineManager* pipelineMgr,
        const gfx::GFXRenderTargetDesc& rtDesc)
    {
        auto program = pb.program;
        if (!program)
            return;

        if (program->GetGpuPrograms().empty())
            return;

        if (!pb.batch.Material->GetShaderConfig())
            return;

        gfx::GFXDescriptorSet* perPassSet = ResolvePerPassSet(pb, reg);
        if (!perPassSet)
            return;

        auto& gpuPrograms = program->GetGpuPrograms();

        gfx::GFXGraphicsPipelineStateParams psoParams{};
        if (effectiveGP)
        {
            psoParams.CullMode          = effectiveGP->CullMode;
            psoParams.DepthCompareOp    = effectiveGP->ZTestOp;
            psoParams.DepthWriteEnable  = effectiveGP->ZWriteEnabled;
            psoParams.DepthTestEnable   = !pb.batch.IsDepthTestDisabled;
            psoParams.StencilTestEnable = effectiveGP->Stencil_Enabled;
            psoParams.BlendEnable       = effectiveGP->Blend_Enabled;
            psoParams.BlendSrcColor     = effectiveGP->Blend_Src;
            psoParams.BlendDstColor     = effectiveGP->Blend_Dst;
            psoParams.BlendSrcAlpha     = effectiveGP->Blend_SrcAlpha;
            psoParams.BlendDstAlpha     = effectiveGP->Blend_DstAlpha;
        }
        if (pb.batch.IsDepthTestDisabled)
        {
            psoParams.DepthTestEnable  = false;
            psoParams.DepthWriteEnable = false;
        }

        auto* resMgr = Application::GetGfxApp()->GetResourceManager();

        array_list<gfx::GFXDescriptorSetLayout_sp> descLayouts;
        descLayouts.push_back(pb.set0Layout);
        descLayouts.push_back(perPassSet->GetDescriptorSetLayout());
        descLayouts.push_back(pb.batch.DescriptorSetLayout);

        auto gfxPipeline = pipelineMgr->GetGraphicsPipeline(
            gpuPrograms, psoParams, descLayouts,
            rtDesc, pb.batch.State);

        if (!gfxPipeline)
            return;

        cmdBuffer.CmdBindGraphicsPipeline(gfxPipeline.get());
        cmdBuffer.CmdSetCullMode(pb.batch.GetCullMode(psoParams.CullMode));

        uint32_t dynOffset = pb.batch.RenderObjectIndex * sizeof(PerRenderObjectData);
        array_list<uint32_t> dynOffsets = { dynOffset };

        for (const auto& element : pb.batch.Elements)
        {
            array_list<gfx::GFXDescriptorSet*> descSets;
            descSets.push_back(pb.set0);
            descSets.push_back(perPassSet);
            descSets.push_back(pb.batch.ExtraDescriptorSet.get());
            cmdBuffer.CmdBindDescriptorSets(descSets, gfxPipeline.get(), &dynOffsets);

            auto* vertBuffer = resMgr->GetBuffer(element.Vertex);
            if (!vertBuffer)
                continue;

            cmdBuffer.CmdBindVertexBuffers({vertBuffer});
            if (pb.batch.IsUsedIndices)
            {
                auto* indicesBuffer = resMgr->GetBuffer(element.Indices);
                if (indicesBuffer)
                {
                    cmdBuffer.CmdBindIndexBuffer(indicesBuffer);
                    uint32_t drawCount = element.DrawCount != 0 ? element.DrawCount : indicesBuffer->GetElementCount();
                    cmdBuffer.CmdDrawIndexed(drawCount);
                }
                else
                {
                    uint32_t drawCount = element.DrawCount != 0 ? element.DrawCount : vertBuffer->GetElementCount();
                    cmdBuffer.CmdDraw(drawCount);
                }
            }
            else
            {
                uint32_t drawCount = element.DrawCount != 0 ? element.DrawCount : vertBuffer->GetElementCount();
                cmdBuffer.CmdDraw(drawCount);
            }
        }
    }

} // namespace pulsar
