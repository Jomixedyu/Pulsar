#include "TranslucencyPass.h"
#include <Pulsar/Components/CameraComponent.h>
#include <Pulsar/World.h>
#include <Pulsar/Scene.h>
#include <Pulsar/Rendering/RenderObject.h>
#include <Pulsar/Rendering/ShaderPass.h>
#include <Pulsar/Rendering/PerPassResources.h>
#include <Pulsar/Assets/Material.h>
#include <Pulsar/Node.h>
#include <gfx/GFXCommandBuffer.h>
#include <gfx/GFXApplication.h>
#include <gfx/GFXGraphicsPipelineManager.h>

namespace pulsar
{
    struct PreparedBatch
    {
        rendering::MeshBatch batch;
        const MaterialPassBinding* binding = nullptr;
    };

    void TranslucencyPass::Initialize(PerPassResources* perPass)
    {
        m_perPassSet = perPass->AllocateSet(perPass->GetLayout("Forward"));
    }

    void TranslucencyPass::Destroy()
    {
        m_perPassSet.reset();
    }

    RGTextureHandle TranslucencyPass::AddToGraph(RenderGraph& graph, RGTextureHandle hFinal,
                                                 CameraComponent* cam, World* world,
                                                 PerPassResources* perPass,
                                                 RGTextureHandle hOpaqueColor,
                                                 RGTextureHandle hOpaqueDepth)
    {
        auto preparedTransparent = std::make_shared<array_list<PreparedBatch>>();

        auto& passBuilder = graph.AddPass("TranslucencyPass")
            .Read(hOpaqueColor)
            .Write(hFinal, RGAttachmentDesc{
                .colorLoadOp  = gfx::GFXRenderPassLoadOp::Load,
                .colorStoreOp = gfx::GFXRenderPassStoreOp::Store,
                .depthLoadOp  = gfx::GFXRenderPassLoadOp::Load,
                .depthStoreOp = gfx::GFXRenderPassStoreOp::DontCare,
            })
            .WithPerPass(perPass);

        if (hOpaqueDepth.IsValid())
        {
            passBuilder.Read(hOpaqueDepth);
        }

        passBuilder.Prepare([perPass, cam, world, preparedTransparent, hOpaqueColor, this](RGPassContext& ctx)
        {
            const Vector3f camPos     = cam->GetNode()->GetTransform()->GetWorldPosition();
            const Vector3f camForward = cam->GetNode()->GetTransform()->GetForward();

            for (const rendering::RenderObject_sp& ro : world->GetRenderObjects())
            {
                const float depth = jmath::Dot(camForward, ro->GetWorldPosition() - camPos);
                for (auto batch : ro->GetMeshBatches())
                {
                    if (batch.Queue != ShaderPassRenderQueueType::Transparency)
                        continue;

                    batch.Depth = depth;
                    const MaterialPassBinding* binding = batch.Material
                        ? batch.Material->PrepareForRendering("Forward", batch.Interface)
                        : nullptr;

                    PreparedBatch pb{ std::move(batch), binding };
                    preparedTransparent->push_back(std::move(pb));
                }
            }

            auto sortDesc = [](const PreparedBatch& a, const PreparedBatch& b)
            {
                if (a.batch.Priority != b.batch.Priority) return a.batch.Priority < b.batch.Priority;
                return a.batch.Depth > b.batch.Depth;
            };
            std::sort(preparedTransparent->begin(), preparedTransparent->end(), sortDesc);

            // Update per-pass textures for this pass's own descriptor set
            perPass->WriteStandardBuffers(this->m_perPassSet.get());

            gfx::GFXTexture2DView* colorView = nullptr;
            if (auto* colorRT = ctx.Get(hOpaqueColor))
                colorView = colorRT->GetGfxRenderTarget0().get();
            perPass->WriteTexture(this->m_perPassSet.get(), 3, colorView);

            perPass->Submit(this->m_perPassSet.get());
        })
        .Execute([perPass, cam, preparedTransparent, this]
                 (RGPassContext& ctx, gfx::GFXCommandBuffer& cmdBuffer)
        {

            auto* targetFBO = cmdBuffer.GetFrameBuffer();
            if (!targetFBO) return;

            auto* pipelineMgr = cmdBuffer.GetApplication()->GetGraphicsPipelineManager();
            cmdBuffer.CmdSetViewport(0, 0, (float)targetFBO->GetWidth(), (float)targetFBO->GetHeight());

            for (const auto& pb : *preparedTransparent)
            {
                if (!pb.binding) continue;

                auto program = pb.binding->GetCurrentProgram();
                if (!program) continue;

                auto shader = pb.batch.Material->GetShader();
                if (!shader || !shader->GetConfig()) continue;

                auto& gpuPrograms = program->GetGpuPrograms();
                auto shaderConfig = shader->GetConfig();

                gfx::GFXGraphicsPipelineStateParams psoParams{};
                if (shaderConfig->Passes && shaderConfig->Passes->size() > 0)
                {
                    auto& passConfig = (*shaderConfig->Passes)[0];
                    if (passConfig->GraphicsPipeline)
                    {
                        auto& gp = passConfig->GraphicsPipeline;
                        psoParams.CullMode          = gp->CullMode;
                        psoParams.DepthCompareOp    = gp->ZTestOp;
                        psoParams.DepthWriteEnable  = gp->ZWriteEnabled;
                        psoParams.DepthTestEnable   = !pb.batch.IsDepthTestDisabled;
                        psoParams.StencilTestEnable = gp->Stencil_Enabled;
                    }
                }
                if (pb.batch.IsDepthTestDisabled)
                {
                    psoParams.DepthTestEnable  = false;
                    psoParams.DepthWriteEnable = false;
                }

                array_list<gfx::GFXDescriptorSetLayout_sp> descLayouts;
                descLayouts.push_back(pb.binding->m_descriptorSetLayout);
                descLayouts.push_back(this->m_perPassSet->GetDescriptorSetLayout());
                descLayouts.push_back(pb.batch.DescriptorSetLayout);

                auto gfxPipeline = pipelineMgr->GetGraphicsPipeline(
                    gpuPrograms, psoParams, descLayouts,
                    targetFBO->GetRenderTargetDesc(), pb.batch.State);

                cmdBuffer.CmdBindGraphicsPipeline(gfxPipeline.get());
                cmdBuffer.CmdSetCullMode(pb.batch.GetCullMode());

                for (const auto& element : pb.batch.Elements)
                {
                    array_list<gfx::GFXDescriptorSet*> descSets;
                    descSets.push_back(pb.binding->m_descriptorSet.get());
                    descSets.push_back(this->m_perPassSet.get());
                    descSets.push_back(element.ModelDescriptor.get());
                    cmdBuffer.CmdBindDescriptorSets(descSets, gfxPipeline.get());

                    cmdBuffer.CmdBindVertexBuffers({element.Vertex.get()});
                    if (pb.batch.IsUsedIndices)
                    {
                        cmdBuffer.CmdBindIndexBuffer(element.Indices.get());
                        cmdBuffer.CmdDrawIndexed(element.Indices->GetElementCount());
                    }
                    else
                    {
                        cmdBuffer.CmdDraw(element.Vertex->GetElementCount());
                    }
                }
            }
        });

        return hFinal;
    }
}
