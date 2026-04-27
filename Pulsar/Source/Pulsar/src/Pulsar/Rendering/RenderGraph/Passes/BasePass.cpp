#include "BasePass.h"
#include <Pulsar/Components/CameraComponent.h>
#include <Pulsar/World.h>
#include <Pulsar/Scene.h>
#include <Pulsar/Rendering/RenderObject.h>
#include <Pulsar/Rendering/ShaderPass.h>
#include <Pulsar/Assets/Material.h>
#include <Pulsar/Node.h>
#include <gfx/GFXCommandBuffer.h>
#include <gfx/GFXApplication.h>
#include <gfx/GFXGraphicsPipelineManager.h>
#include <Pulsar/Rendering/PerPassResources.h>

namespace pulsar
{
    struct PreparedBatch
    {
        rendering::MeshBatch batch;
        const MaterialPassBinding* binding = nullptr;
    };

    RGTextureHandle BasePass::AddToGraph(RenderGraph& graph, RGTextureHandle hFinal,
                                         CameraComponent* cam, World* world,
                                         PerPassResources* perPass,
                                         gfx::GFXTexture2DView* resolveTargetView)
    {
        auto preparedOpaque      = std::make_shared<array_list<PreparedBatch>>();
        auto preparedAlphaTest   = std::make_shared<array_list<PreparedBatch>>();
        auto preparedTransparent = std::make_shared<array_list<PreparedBatch>>();

        graph.AddPass("BasePass")
            .Write(hFinal, RGAttachmentDesc{
                .colorLoadOp  = gfx::GFXRenderPassLoadOp::Clear,
                .colorStoreOp = resolveTargetView ? gfx::GFXRenderPassStoreOp::DontCare : gfx::GFXRenderPassStoreOp::Store,
                .clearColor   = {0.3f, 0.3f, 0.3f, 1.f},
                .depthLoadOp  = gfx::GFXRenderPassLoadOp::Clear,
                .depthStoreOp = gfx::GFXRenderPassStoreOp::DontCare,
                .clearDepth   = 1.f,
                .resolveTargetView = resolveTargetView,
            })
            .WithPerPass(perPass)
            .Prepare([cam, world, preparedOpaque, preparedAlphaTest, preparedTransparent](RGPassContext&)
            {
                const Vector3f camPos     = cam->GetNode()->GetTransform()->GetWorldPosition();
                const Vector3f camForward = cam->GetNode()->GetTransform()->GetForward();

                for (const rendering::RenderObject_sp& ro : world->GetRenderObjects())
                {
                    const float depth = jmath::Dot(camForward, ro->GetWorldPosition() - camPos);
                    for (auto batch : ro->GetMeshBatches())
                    {
                        batch.Depth = depth;
                        const MaterialPassBinding* binding = batch.Material
                            ? batch.Material->PrepareForRendering("Forward", batch.Interface)
                            : nullptr;

                        PreparedBatch pb{ std::move(batch), binding };
                        switch (pb.batch.Queue)
                        {
                        case ShaderPassRenderQueueType::AlphaTest:
                            preparedAlphaTest->push_back(std::move(pb));
                            break;
                        case ShaderPassRenderQueueType::Transparency:
                            preparedTransparent->push_back(std::move(pb));
                            break;
                        default:
                            preparedOpaque->push_back(std::move(pb));
                            break;
                        }
                    }
                }

                auto sortAsc  = [](const PreparedBatch& a, const PreparedBatch& b)
                {
                    if (a.batch.Priority != b.batch.Priority) return a.batch.Priority < b.batch.Priority;
                    return a.batch.Depth < b.batch.Depth;
                };
                auto sortDesc = [](const PreparedBatch& a, const PreparedBatch& b)
                {
                    if (a.batch.Priority != b.batch.Priority) return a.batch.Priority < b.batch.Priority;
                    return a.batch.Depth > b.batch.Depth;
                };
                std::sort(preparedOpaque->begin(),      preparedOpaque->end(),      sortAsc);
                std::sort(preparedAlphaTest->begin(),   preparedAlphaTest->end(),   sortAsc);
                std::sort(preparedTransparent->begin(), preparedTransparent->end(), sortDesc);
            })
            .Execute([cam, perPass, preparedOpaque, preparedAlphaTest, preparedTransparent]
                     (RGPassContext& ctx, gfx::GFXCommandBuffer& cmdBuffer)
            {
                auto* targetFBO = cmdBuffer.GetFrameBuffer();
                if (!targetFBO) return;

                auto* pipelineMgr = cmdBuffer.GetApplication()->GetGraphicsPipelineManager();
                cmdBuffer.CmdSetViewport(0, 0, (float)targetFBO->GetWidth(), (float)targetFBO->GetHeight());

                auto DrawBatchList = [&](const array_list<PreparedBatch>& entries)
                {
                    for (const auto& pb : entries)
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
                        descLayouts.push_back(perPass->GetDescriptorSetLayout());
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
                            descSets.push_back(perPass->GetDescriptorSet().get());
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
                };

                DrawBatchList(*preparedOpaque);
                DrawBatchList(*preparedAlphaTest);
                DrawBatchList(*preparedTransparent);
            });

        return hFinal;
    }
}
