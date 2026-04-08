#include "DefaultSceneCaptureRenderer.h"
#include <Pulsar/Logger.h>
#include <Pulsar/Node.h>
#include <Pulsar/Components/CameraComponent.h>
#include <Pulsar/Components/SceneCaptureComponent.h>
#include <Pulsar/World.h>
#include <Pulsar/Scene.h>
#include <Pulsar/Rendering/RenderObject.h>
#include <Pulsar/Rendering/ShaderPass.h>
#include <Pulsar/Assets/Material.h>
#include <Pulsar/Application.h>
#include <gfx/GFXCommandBuffer.h>
#include <gfx/GFXApplication.h>
#include <gfx/GFXGraphicsPipelineManager.h>
#include "../PerPassResources.h"
#include "../LightingData.h"

namespace pulsar
{
    DefaultSceneCaptureRenderer::DefaultSceneCaptureRenderer()
    {
        m_perPassResources.Initialize();
    }

    DefaultSceneCaptureRenderer::~DefaultSceneCaptureRenderer()
    {
        m_perPassResources.Destroy();
    }

    void DefaultSceneCaptureRenderer::Render(RenderGraph& graph, const RenderCaptureContext& ctx)
    {
        auto* cam   = dynamic_cast<CameraComponent*>(ctx.capture);
        auto* world = ctx.world;

        if (!cam || !world)
        {
            Logger::Log("DefaultSceneCaptureRenderer: invalid capture context (cam or world is null)", LogLevel::Error);
            return;
        }

        auto* camRenderTexture = cam->GetRenderTexture().GetPtr();
        if (!camRenderTexture)
        {
            Logger::Log("DefaultSceneCaptureRenderer: camera has no render texture", LogLevel::Error);
            return;
        }

        auto* perPass = &m_perPassResources;

        PerPassCameraData camData{};
        camData.MatrixV     = cam->GetViewMat();
        camData.MatrixP     = cam->GetProjectionMat();
        camData.MatrixVP    = camData.MatrixP * camData.MatrixV;
        camData.InvMatrixV  = jmath::Inverse(camData.MatrixV);
        camData.InvMatrixP  = jmath::Inverse(camData.MatrixP);
        camData.InvMatrixVP = jmath::Inverse(camData.MatrixVP);
        camData.CamPosition = Vector4f(cam->GetNode()->GetTransform()->GetWorldPosition(), 1.f);
        camData.CamNear     = cam->GetNear();
        camData.CamFar      = cam->GetFar();
        camData.Resolution  = cam->GetRenderTexture()->GetSize2df();
        perPass->UpdateCamera(camData);

        {
            PerPassWorldData worldData{};
            worldData.TotalTime  = world->GetTotalTime();
            worldData.DeltaTime  = world->GetTicker().deltatime;

            if (auto* env = world->GetFocusScene()->GetRuntimeEnvironment())
            {
                if (const auto* dirLight = env->GetDirectionalLight())
                {
                    worldData.WorldSpaceLightVector = -dirLight->Vector;
                    auto& c = dirLight->Color;
                    worldData.WorldSpaceLightColor  = {c.r, c.g, c.b, dirLight->Intensity};
                }
                {
                    auto skyLight = env->GetSkyLight();
                    worldData.SkyLightColor = {skyLight.Color.r, skyLight.Color.g, skyLight.Color.b, skyLight.Intensity};
                }
            }
            worldData.LightParameterCount = static_cast<uint32_t>(world->GetLightManager()->GetLightCount());
            perPass->UpdateWorld(worldData);
        }

        {
            PerPassLightsBufferData lightsData{};
            auto* lightMgr = ctx.world->GetLightManager();
            int lightCount = std::min(lightMgr->GetLightCount(), 63);
            for (int i = 0; i < lightCount; ++i)
            {
                auto& src = lightMgr->GetLightParameter(i);
                lightsData.Lights[i] = src;
            }
            perPass->UpdateLights(lightsData);
        }

        perPass->Submit();

        RGTextureHandle hFinal = graph.ImportTexture("FinalOutput", camRenderTexture);

        // PreparedBatch: batch + its ready binding (nullptr if shader not yet compiled)
        struct PreparedBatch
        {
            rendering::MeshBatch       batch;
            const MaterialPassBinding* binding = nullptr;
        };
        auto preparedOpaque      = std::make_shared<array_list<PreparedBatch>>();
        auto preparedAlphaTest   = std::make_shared<array_list<PreparedBatch>>();
        auto preparedTransparent = std::make_shared<array_list<PreparedBatch>>();

        graph.AddPass("BasePass")
            .Write(hFinal, RGAttachmentDesc{
                .colorLoadOp  = gfx::GFXRenderPassLoadOp::Clear,
                .colorStoreOp = gfx::GFXRenderPassStoreOp::Store,
                .clearColor   = {0.3f, 0.3f, 0.3f, 1.f},
                .depthLoadOp  = gfx::GFXRenderPassLoadOp::Clear,
                .depthStoreOp = gfx::GFXRenderPassStoreOp::DontCare,
                .clearDepth   = 1.f,
            })
            .WithPerPass(perPass)
            .Prepare([cam, world, preparedOpaque, preparedAlphaTest, preparedTransparent](RGPassContext&)
            {
                // Collect batches and prepare material GPU resources.
                // This runs after Compile() but before BeginRenderPass — safe for resource creation.
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
                auto* targetFBO = cam->GetRenderTexture()->GetGfxFrameBufferObject().get();
                if (!targetFBO) return;

                auto* pipelineMgr = cmdBuffer.GetApplication()->GetGraphicsPipelineManager();
                cmdBuffer.CmdSetViewport(0, 0, (float)targetFBO->GetWidth(), (float)targetFBO->GetHeight());

                auto DrawBatchList = [&](const array_list<PreparedBatch>& entries)
                {
                    for (const auto& pb : entries)
                    {
                        if (!pb.binding) continue; // shader not ready yet

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
                                psoParams.DepthTestEnable   = true;
                                psoParams.StencilTestEnable = gp->Stencil_Enabled;
                            }
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

        // ---- Post-Process Passes ----
        // Collect valid post-process materials from the capture component
        const size_t ppCount = ctx.capture ? ctx.capture->GetPostProcessCount() : 0;

        if (ppCount > 0)
        {
            auto* camRT  = cam->GetRenderTexture().GetPtr();

            RGTextureDesc pingPongDesc{};
            pingPongDesc.Width  = camRT->GetWidth();
            pingPongDesc.Height = camRT->GetHeight();
            auto format = camRT->GetRenderTargets()[0]->GetFormat();
            pingPongDesc.TargetInfos.push_back({ gfx::GFXTextureTargetType::ColorTarget, format });

            {
            RGTextureHandle hPingPong = graph.CreateTransient("PostProcessPingPong", pingPongDesc);

            // hSrc: 当前读取源（初始为 BasePass 的输出 hFinal）
            // hDst: 当前写入目标（初始为 hPingPong）
            // 每个 Pass 结束后交换 hSrc/hDst，绝不在 Read 和 Write 使用同一张纹理。
            // 若最终结果落在 hPingPong（奇数个 Pass），额外追加一个 CopyToFinal Pass。
            RGTextureHandle hSrc = hFinal;
            RGTextureHandle hDst = hPingPong;

            size_t passIdx = 0;
            for (size_t i = 0; i < ppCount; ++i)
            {
                RCPtr<Material> ppMat = ctx.capture->GetPostprocess(i);
                if (!ppMat) continue;

                const size_t curIdx = passIdx++;
                std::string passName = "PostProcessMaterial_" + std::to_string(curIdx);

                // 在 build 阶段提前获取或创建 per-Renderer (set2) 资源（layout+set），
                // 缓存到 m_ppRendererCache，避免在 execute lambda 里每帧动态分配。
                // 以 curIdx 为下标（不以材质指针为 key，避免材质析构后地址复用导致缓存污染）
                if (curIdx >= m_ppRendererCache.size())
                    m_ppRendererCache.resize(curIdx + 1);
                auto& ppRes = m_ppRendererCache[curIdx];
                if (!ppRes.layout)
                {
                    auto* gfxApp = Application::GetGfxApp();
                    gfx::GFXDescriptorSetLayoutDesc desc{
                        gfx::GFXDescriptorType::CombinedImageSampler,
                        gfx::GFXGpuProgramStageFlags::Fragment,
                        0, 1
                    };
                    ppRes.layout = gfxApp->CreateDescriptorSetLayout(&desc, 1);
                    ppRes.set    = gfxApp->GetDescriptorManager()->GetDescriptorSet(ppRes.layout);
                }

                auto* ppRendererSet    = ppRes.set.get();
                auto  ppRendererLayout = ppRes.layout;

                graph.AddPass(passName)
                    .Read(hSrc)
                    .Write(hDst, RGAttachmentDesc{
                        .colorLoadOp  = gfx::GFXRenderPassLoadOp::DontCare,
                        .colorStoreOp = gfx::GFXRenderPassStoreOp::Store,
                    })
                    .WithPerPass(perPass)
                    .Prepare([ppMat, ppRendererSet](RGPassContext&)
                    {
                        // PrepareForRendering must run before BeginRenderPass (resource creation + descriptor updates)
                        if (ppMat)
                            ppMat->PrepareForRendering("PostProcess", "RENDERER_IMAGEPROCESS");
                        (void)ppRendererSet;
                    })
                    .Execute([ppMat, hSrc, hDst, hFinal, cam, perPass, ppRendererSet, ppRendererLayout](RGPassContext& passCtx, gfx::GFXCommandBuffer& cmdBuffer)
                    {
                        auto* mat = ppMat.GetPtr();
                        if (!mat) return;

                        auto shader = mat->GetShader();
                        if (!shader || !shader->GetConfig()) return;

                        // set 0: per-Material (user params)
                        // set 1: per-Pass (camera / lights)
                        // set 2: per-Renderer — PP_InColor (t0/s0, space2)
                        // PrepareForRendering was already called in Prepare; here we just fetch the result.
                        const auto* ppPassBinding = mat->GetPassBinding("PostProcess", "RENDERER_IMAGEPROCESS")
                                                        .m_gpuResourcesInitialized
                                                    ? &mat->GetPassBinding("PostProcess", "RENDERER_IMAGEPROCESS")
                                                    : nullptr;
                        if (!ppPassBinding) return;

                        auto program = ppPassBinding->GetCurrentProgram();
                        if (!program) return;

                        if (!ppRendererSet) return;

                        // ping-pong RT 只有 color，FBO 在 execute 时可以正常取到
                        // hDst 是 transient，通过 passCtx.Get 取；若是 hFinal（imported）则直接用 cam RT
                        RenderTexture* dstRT = passCtx.Get(hDst);
                        if (!dstRT && hDst == hFinal)
                            dstRT = cam->GetRenderTexture().GetPtr();
                        if (!dstRT) return;
                        auto* dstFBO = dstRT->GetGfxFrameBufferObject().get();
                        if (!dstFBO) return;

                        // 每帧更新 PP_InColor 绑定（场景颜色来源变化时需要重新 Submit）
                        // hSrc 首次是 hFinal（imported），passCtx.Get 可能对 imported 资源返回 null，fallback 到 cam RT
                        RenderTexture* srcRT = passCtx.Get(hSrc);
                        if (!srcRT && hSrc == hFinal)
                            srcRT = cam->GetRenderTexture().GetPtr();
                        if (srcRT)
                        {
                            auto srcView = srcRT->GetGfxRenderTarget0();
                            if (srcView)
                            {
                                // FindByBinding 找已有的 descriptor，没有才 Add，避免每帧重复追加
                                auto* desc = ppRendererSet->FindByBinding(0);
                                if (!desc)
                                    desc = ppRendererSet->AddDescriptor("PP_InColor", 0);
                                desc->SetTextureSampler2D(srcView.get());
                            }
                        }
                        ppRendererSet->Submit();

                        auto* gfxApp          = cmdBuffer.GetApplication();
                        auto* pipelineMgr     = gfxApp->GetGraphicsPipelineManager();
                        array_list<gfx::GFXDescriptorSetLayout_sp> descLayouts;
                        descLayouts.push_back(ppPassBinding->m_descriptorSetLayout); // set 0
                        descLayouts.push_back(perPass->GetDescriptorSetLayout());   // set 1
                        descLayouts.push_back(ppRendererLayout);                    // set 2

                        auto& gpuPrograms = program->GetGpuPrograms();

                        gfx::GFXGraphicsPipelineStateParams psoParams{};
                        psoParams.DepthTestEnable  = false;
                        psoParams.DepthWriteEnable = false;
                        psoParams.CullMode         = gfx::GFXCullMode::None;

                        auto gfxPipeline = pipelineMgr->GetGraphicsPipeline(
                            gpuPrograms, psoParams, descLayouts,
                            dstFBO->GetRenderTargetDesc(), {});
                        if (!gfxPipeline) return;

                        cmdBuffer.CmdSetViewport(0, 0, (float)dstFBO->GetWidth(), (float)dstFBO->GetHeight());
                        cmdBuffer.CmdBindGraphicsPipeline(gfxPipeline.get());

                        array_list<gfx::GFXDescriptorSet*> descSets;
                        descSets.push_back(ppPassBinding->m_descriptorSet.get());   // set 0
                        descSets.push_back(perPass->GetDescriptorSet().get());      // set 1
                        descSets.push_back(ppRendererSet);                          // set 2
                        cmdBuffer.CmdBindDescriptorSets(descSets, gfxPipeline.get());

                        // Full-screen triangle
                        cmdBuffer.CmdDraw(3);
                    });

                // 每次 pass 后交换，保证下一个 pass 的 Read 和 Write 始终是不同纹理
                std::swap(hSrc, hDst);
            }

            // 经过奇数个 pass 后，最终结果在 hPingPong（此时 hSrc == hPingPong）
            // 需要再做一次 Blit 把结果写回 hFinal
            if (hSrc != hFinal)
            {
                // hFinal 是 Write 资源，passCtx.Get 取不到；提前拿好 dstView
//                auto* camFinalRT  = cam->GetRenderTexture().GetPtr();
//                auto  capturedDstView = camFinalRT ? camFinalRT->GetGfxRenderTarget0() : nullptr;

                // CopyToFinal is a transfer-only (blit) pass.
                // Write(hFinal) is declared for correct resource barrier and lifetime tracking,
                // but NoRenderPass() prevents BeginRenderPass/EndRenderPass from being called,
                // so CmdBlit (and its internal layout transitions) run safely outside a render pass.
                graph.AddPass("PostProcess_CopyToFinal")
                    .Read(hSrc)
                    .Write(hFinal)
                    .NoRenderPass()
                    .Execute([hSrc, hFinal, cam](RGPassContext& passCtx, gfx::GFXCommandBuffer& cmdBuffer)
                    {
                        auto* srcRT   = passCtx.Get(hSrc);
                        // hFinal is imported; passCtx.Get works even without declaring Write(hFinal)
                        auto* finalRT = passCtx.Get(hFinal);
                        if (!finalRT)
                            finalRT = cam->GetRenderTexture().GetPtr();
                        if (!srcRT || !finalRT) return;

                        auto srcView   = srcRT->GetGfxRenderTarget0();
                        auto finalView = finalRT->GetGfxRenderTarget0();
                        if (!srcView || !finalView) return;

                        cmdBuffer.CmdBlit(srcView.get(), finalView.get());
                    });
            }
            }
        }
    }

} // namespace pulsar
