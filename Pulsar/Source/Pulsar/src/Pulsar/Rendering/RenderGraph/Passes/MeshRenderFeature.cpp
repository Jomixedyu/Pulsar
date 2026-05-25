#include "MeshRenderFeature.h"
#include <Pulsar/Rendering/PerPassResources.h>
#include <Pulsar/Application.h>

namespace pulsar
{
    void MeshRenderFeature::Initialize(PerPassResources* perPass)
    {
        m_perPassSet = perPass->AllocateSet(perPass->GetLayout(GetPerPassLayoutName()));
    }

    void MeshRenderFeature::Destroy()
    {
        m_perPassSet.reset();
    }

    void MeshRenderFeature::DrawPreparedBatch(
        gfx::GFXCommandBuffer& cmdBuffer,
        const PreparedBatch& pb,
        gfx::GFXDescriptorSet* perPassSet,
        const SPtr<ShaderConfigGraphicsPipeline>& effectiveGP,
        gfx::GFXGraphicsPipelineManager* pipelineMgr,
        const gfx::GFXRenderTargetDesc& rtDesc)
    {
        auto program = pb.binding->GetCurrentProgram();
        if (!program)
            return;

        auto shader = pb.batch.Material->GetShader();
        if (!shader || !shader->GetConfig())
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
        descLayouts.push_back(pb.binding->m_descriptorSetLayout.Lock());
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
            descSets.push_back(pb.binding->m_descriptorSet.get());
            descSets.push_back(perPassSet);
            descSets.push_back(pb.batch.ExtraDescriptorSet.get());
            cmdBuffer.CmdBindDescriptorSets(descSets, gfxPipeline.get(), &dynOffsets);

            if (!element.Vertex.IsValid())
                continue;

            cmdBuffer.CmdBindVertexBuffers({element.Vertex.Get()});
            if (pb.batch.IsUsedIndices)
            {
                if (element.Indices.IsValid())
                {
                    cmdBuffer.CmdBindIndexBuffer(element.Indices.Get());
                    cmdBuffer.CmdDrawIndexed(element.Indices->GetElementCount());
                }
                else
                {
                    cmdBuffer.CmdDraw(element.Vertex->GetElementCount());
                }
            }
            else
            {
                cmdBuffer.CmdDraw(element.Vertex->GetElementCount());
            }
        }
    }

} // namespace pulsar
