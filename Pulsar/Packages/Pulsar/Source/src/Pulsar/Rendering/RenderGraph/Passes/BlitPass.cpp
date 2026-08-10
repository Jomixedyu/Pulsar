#include "BlitPass.h"
#include <Pulsar/Rendering/DescriptorSetAssembler.h>
#include <Pulsar/Rendering/DescriptorSetCache.h>
#include <Pulsar/Rendering/RenderResourceRegistry.h>
#include <gfx/GFXApplication.h>
#include <gfx/GFXCommandBuffer.h>
#include <gfx/GFXFrameBufferObject.h>
#include <gfx/GFXGraphicsPipelineManager.h>
#include <gfx/GFXTexture.h>

namespace pulsar
{
    RGTextureHandle BlitPass::AddToGraph(RenderGraph& graph, Desc desc)
    {
        if (!desc.Source.IsValid() || !desc.Destination.IsValid() || !desc.Material)
            return desc.Source;

        graph.AddPass(desc.Name)
            .Read(desc.Source)
            .Write(desc.Destination, desc.Attachment)
            .Prepare([material = desc.Material, shaderPass = desc.ShaderPass, interfaceName = desc.Interface](RGPassContext&)
            {
                material->ResolveRenderVariant(shaderPass, interfaceName);
            })
            .Execute([desc = std::move(desc)](RGPassContext& passCtx, gfx::GFXCommandBuffer& cmdBuffer)
            {
                auto resolved = desc.Material->ResolveRenderVariant(desc.ShaderPass, desc.Interface);
                if (!resolved)
                    return;

                auto program = resolved.m_program;
                if (!program || program->GetGpuPrograms().empty())
                    return;

                const auto* dstRT = passCtx.Get(desc.Destination);
                if (!dstRT)
                    return;

                auto* dstFBO = dstRT->GetFrameBufferObject().get();
                if (!dstFBO)
                    return;

                gfx::GFXTexture2DView* srcView = nullptr;
                if (const auto* srcRT = passCtx.Get(desc.Source))
                {
                    if (auto view = srcRT->GetRenderTarget0())
                        srcView = view.get();
                }

                RenderResourceRegistry reg;
                reg.Set(desc.SourceBindingName, srcView);
                reg.Set("CameraBuffer", desc.CameraBuffer);
                reg.Set("WorldBuffer", desc.WorldBuffer);

                const ShaderPropertySetLayout* set1 = program->m_layout.FindSet(1);
                auto set1Layout = DescriptorSetAssembler::BuildLayout(set1);
                gfx::GFXDescriptorSet* perPassSet = DescriptorSetCache::Instance().Get(set1Layout, set1, reg);
                if (!perPassSet)
                    return;

                auto* gfxApp = cmdBuffer.GetApplication();
                auto* pipelineMgr = gfxApp->GetGraphicsPipelineManager();

                array_list<gfx::GFXDescriptorSetLayout_sp> descLayouts;
                descLayouts.push_back(resolved.m_set0Layout);
                descLayouts.push_back(set1Layout);

                gfx::GFXGraphicsPipelineStateParams psoParams{};
                psoParams.DepthTestEnable = false;
                psoParams.DepthWriteEnable = false;
                psoParams.CullMode = gfx::GFXCullMode::None;

                auto gfxPipeline = pipelineMgr->GetGraphicsPipeline(
                    program->GetGpuPrograms(), psoParams, descLayouts,
                    dstFBO->GetRenderTargetDesc(), {});
                if (!gfxPipeline)
                    return;

                cmdBuffer.CmdSetViewport(0, 0, static_cast<float>(dstFBO->GetWidth()), static_cast<float>(dstFBO->GetHeight()));
                cmdBuffer.CmdBindGraphicsPipeline(gfxPipeline.get());

                array_list<gfx::GFXDescriptorSet*> descSets;
                descSets.push_back(resolved.m_set0);
                descSets.push_back(perPassSet);
                cmdBuffer.CmdBindDescriptorSets(descSets, gfxPipeline.get());
                cmdBuffer.CmdDraw(3);
            });

        return desc.Destination;
    }
}