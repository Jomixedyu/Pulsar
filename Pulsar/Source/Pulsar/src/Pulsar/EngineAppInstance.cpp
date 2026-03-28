#include "EngineAppInstance.h"
#include "Assets/StaticMesh.h"
#include <algorithm>
#include "Components/CameraComponent.h"
#include "Components/StaticMeshRendererComponent.h"
#include "Node.h"
#include "Rendering/LightingData.h"
#include "Rendering/PerPassResources.h"
#include "Rendering/RenderObject.h"
#include "Scene.h"

#include <Pulsar/Application.h>
#include <Pulsar/EngineAppInstance.h>
#include <Pulsar/ImGuiImpl.h>
#include <Pulsar/Logger.h>
#include <Pulsar/World.h>
#include <Pulsar/Rendering/ShaderInstanceCache.h>
#include <filesystem>

namespace pulsar
{

    EngineRenderPipeline::EngineRenderPipeline(const std::initializer_list<World*>& worlds)
    {
        for (auto world : worlds)
        {
            AddWorld(world);
        }
    }


    void EngineRenderPipeline::OnRender(
        gfx::GFXRenderContext* context, gfx::GFXFrameBufferObject* backbuffer)
    {
        auto& cmdBuffer = context->GetCommandBuffer(0);

        for (auto world : m_worlds)
        {
            cmdBuffer.CmdPushDebugInfo("SceneRenderer");

            auto& renderObjects = world->GetRenderObjects();

            auto pipelineMgr = context->GetApplication()->GetGraphicsPipelineManager();

            for (const auto& cam : world->GetCameraManager().GetCameras())
            {
                auto targetFBO = cam->GetRenderTexture()->GetGfxFrameBufferObject().get();

                // 同步 Camera 数据到 PerPassResources (set 1)
                {
                    PerPassCameraData camData{};
                    camData.MatrixV = cam->GetViewMat();
                    camData.MatrixP = cam->GetProjectionMat();
                    camData.MatrixVP = camData.MatrixP * camData.MatrixV;
                    camData.InvMatrixV = jmath::Inverse(camData.MatrixV);
                    camData.InvMatrixP = jmath::Inverse(camData.MatrixP);
                    camData.InvMatrixVP = jmath::Inverse(camData.MatrixVP);
                    camData.CamPosition = cam->GetNode()->GetTransform()->GetWorldPosition();
                    camData.CamNear = cam->GetNear();
                    camData.CamFar = cam->GetFar();
                    camData.Resolution = cam->GetRenderTexture()->GetSize2df();
                    world->GetPerPassResources().UpdateCamera(camData);
                }

                // 同步 Light 数据到 PerPassResources (set 1)
                {
                    PerPassLightsBufferData lightsData{};
                    auto lightMgr = world->GetLightManager();
                    int lightCount = std::min(lightMgr->GetLightCount(), 63);
                    for (int i = 0; i < lightCount; ++i)
                    {
                        auto& src = lightMgr->GetLightParameter(i);
                        auto& dst = lightsData.Lights[i];
                        dst.Position = {src.WorldPosition.x, src.WorldPosition.y, src.WorldPosition.z};
                        dst.CutOff = src.SpotAngles.x;
                        dst.Direction = {src.Direction.x, src.Direction.y, src.Direction.z};
                        dst.Radius = src.SourceRadius;
                        dst.Color = src.Color;
                    }
                    lightsData.PointCount = static_cast<uint32_t>(lightCount);
                    world->GetPerPassResources().UpdateLights(lightsData);
                }

                world->GetPerPassResources().Submit();

                cmdBuffer.SetFrameBuffer(targetFBO);

                for (auto& rt : targetFBO->GetRenderTargets())
                {
                    cmdBuffer.CmdClearColor(rt->GetTexture());
                }

                cmdBuffer.CmdBeginRenderPass("BasePass");
                cmdBuffer.CmdSetViewport(0, 0, (float)targetFBO->GetWidth(), (float)targetFBO->GetHeight());

                // -------------------------------------------------------
                // 收集 BatchEntry 列表，计算 Depth
                // -------------------------------------------------------
                struct BatchEntry
                {
                    rendering::RenderObject* renderObject;
                    rendering::MeshBatch     batch; // value copy (contains Depth)
                };

                const Vector3f camPos     = cam->GetNode()->GetTransform()->GetWorldPosition();
                const Vector3f camForward = cam->GetNode()->GetTransform()->GetForward();

                array_list<BatchEntry> opaqueList, alphaTestList, transparentList;

                for (const rendering::RenderObject_sp& renderObject : renderObjects)
                {
                    const Vector3f objPos = renderObject->GetWorldPosition();
                    const float depth = jmath::Dot(camForward, objPos - camPos);

                    for (auto batch : renderObject->GetMeshBatches())
                    {
                        batch.Depth = depth;

                        switch (batch.Queue)
                        {
                        case ShaderPassRenderQueueType::AlphaTest:
                            alphaTestList.push_back({renderObject.get(), std::move(batch)});
                            break;
                        case ShaderPassRenderQueueType::Transparency:
                            transparentList.push_back({renderObject.get(), std::move(batch)});
                            break;
                        case ShaderPassRenderQueueType::Opaque:
                        default:
                            opaqueList.push_back({renderObject.get(), std::move(batch)});
                            break;
                        }
                    }
                }

                // -------------------------------------------------------
                // 排序
                // Opaque & AlphaTest: (priority ASC, depth ASC)   近 → 远
                // Transparent:        (priority ASC, depth DESC)   远 → 近
                // -------------------------------------------------------
                auto sortAscDepth = [](const BatchEntry& a, const BatchEntry& b) {
                    if (a.batch.Priority != b.batch.Priority)
                        return a.batch.Priority < b.batch.Priority;
                    return a.batch.Depth < b.batch.Depth;
                };
                auto sortDescDepth = [](const BatchEntry& a, const BatchEntry& b) {
                    if (a.batch.Priority != b.batch.Priority)
                        return a.batch.Priority < b.batch.Priority;
                    return a.batch.Depth > b.batch.Depth;
                };

                std::sort(opaqueList.begin(),      opaqueList.end(),      sortAscDepth);
                std::sort(alphaTestList.begin(),   alphaTestList.end(),   sortAscDepth);
                std::sort(transparentList.begin(), transparentList.end(), sortDescDepth);

                // -------------------------------------------------------
                // 绘制辅助函数
                // -------------------------------------------------------
                auto DrawBatchList = [&](const array_list<BatchEntry>& entries)
                {
                    for (const auto& entry : entries)
                    {
                        const auto& batch = entry.batch;

                        if (!batch.Material)
                            continue;
                        auto shader = batch.Material->GetShader();
                        if (!shader || !shader->GetConfig())
                            continue;

                        const auto& passBinding = batch.Material->GetPassBinding("Forward", batch.Interface);
                        // 每帧提交参数（纹理、cbuffer），EnsureGPUResources 也在此触发
                        batch.Material->SubmitParameters();
                        auto program = passBinding.GetCurrentProgram();
                        if (!program)
                        {
                            Logger::Log("DrawBatchList: program is null (shader still compiling?)", LogLevel::Warning);
                            continue;
                        }

                        auto& gpuPrograms = program->GetGpuPrograms();
                        auto shaderConfig = shader->GetConfig();

                        // 从 ShaderConfigPass 中获取 GraphicsPipeline 参数
                        gfx::GFXGraphicsPipelineStateParams psoParams{};
                        if (shaderConfig->Passes && shaderConfig->Passes->size() > 0)
                        {
                            auto& passConfig = (*shaderConfig->Passes)[0];
                            if (passConfig->GraphicsPipeline)
                            {
                                auto& gp = passConfig->GraphicsPipeline;
                                psoParams.CullMode = gp->CullMode;
                                psoParams.DepthCompareOp = gp->ZTestOp;
                                psoParams.DepthWriteEnable = gp->ZWriteEnabled;
                                psoParams.DepthTestEnable = true;
                                psoParams.StencilTestEnable = gp->Stencil_Enabled;
                            }
                        }

                        // descriptor set layouts: set 0 (material), set 1 (pass), set 2 (renderer)
                        array_list<gfx::GFXDescriptorSetLayout_sp> descriptorSetLayouts;

                        auto matDescSetLayout = batch.Material->GetDescriptorSetLayout();
                        if (!matDescSetLayout)
                        {
                            Logger::Log("DrawBatchList: matDescSetLayout is null, skipping. shader="
                                + (batch.Material->GetShader() ? batch.Material->GetShader()->GetName() : string("null"))
                                + " gpuInit=" + std::to_string(batch.Material->IsCreatedGPUResource()),
                                LogLevel::Warning);
                            continue;
                        }
                        descriptorSetLayouts.push_back(matDescSetLayout);
                        descriptorSetLayouts.push_back(world->GetPerPassResources().GetDescriptorSetLayout());
                        descriptorSetLayouts.push_back(batch.DescriptorSetLayout);

                        auto gfxPipeline = pipelineMgr->GetGraphicsPipeline(
                            gpuPrograms, psoParams, descriptorSetLayouts,
                            targetFBO->GetRenderTargetDesc(), batch.State);
                        cmdBuffer.CmdBindGraphicsPipeline(gfxPipeline.get());
                        cmdBuffer.CmdSetCullMode(batch.GetCullMode());

                        for (const auto& element : batch.Elements)
                        {
                            array_list<gfx::GFXDescriptorSet*> descriptorSets;
                            descriptorSets.push_back(batch.Material->GetDescriptorSet().get());
                            descriptorSets.push_back(world->GetPerPassResources().GetDescriptorSet().get());
                            descriptorSets.push_back(element.ModelDescriptor.get());
                            cmdBuffer.CmdBindDescriptorSets(descriptorSets, gfxPipeline.get());

                            cmdBuffer.CmdBindVertexBuffers({element.Vertex.get()});
                            if (batch.IsUsedIndices)
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

                // -------------------------------------------------------
                // 按序绘制三组：Opaque → AlphaTest → Transparent
                // -------------------------------------------------------
                DrawBatchList(opaqueList);
                DrawBatchList(alphaTestList);
                DrawBatchList(transparentList);

                cmdBuffer.CmdEndRenderPass();
                cmdBuffer.SetFrameBuffer(nullptr);

                // deferred lighting pass


                // post processing
                RCPtr<RenderTexture> lastPPRt;
                auto ppcount = cam->GetPostProcessCount();
                size_t ppCount = 0;
                for (size_t i = 0; i < ppcount; ++i)
                {
                    auto ppMat = cam->GetPostprocess(i);
                    if (!ppMat)
                    {
                        continue;
                    }

                    auto matName = ppMat->GetName();

                    RCPtr<RenderTexture> srcRt;
                    RCPtr<RenderTexture> destRt;
                    gfx::GFXDescriptorSet_sp srcResourceDescSet;
                    if(i % 2 == 1)
                    {
                        srcResourceDescSet = cam->m_postprocessDescA;
                        srcRt = cam->m_postprocessRtB;
                        destRt = cam->m_postprocessRtA;
                    }
                    else //first
                    {
                        srcResourceDescSet = cam->m_postprocessDescB;
                        srcRt = cam->m_postprocessRtA;
                        destRt = cam->m_postprocessRtB;
                    }
                    lastPPRt = destRt;

                    if (i == 0)
                    {
                        cmdBuffer.CmdBlit(cam->GetRenderTexture()->GetGfxRenderTarget0().get(), srcRt->GetGfxRenderTarget0().get());
                    }
                    cmdBuffer.CmdImageTransitionBarrier(srcRt->GetGfxRenderTarget0().get(), gfx::GFXResourceLayout::ShaderReadOnly);
                    cmdBuffer.CmdImageTransitionBarrier(destRt->GetGfxRenderTarget0().get(), gfx::GFXResourceLayout::RenderTarget);

                    cmdBuffer.SetFrameBuffer(destRt->GetGfxFrameBufferObject().get());
                    cmdBuffer.CmdBeginRenderPass("PostProcessingPass-" + matName);
                    cmdBuffer.CmdSetViewport(0, 0, (float)targetFBO->GetWidth(), (float)targetFBO->GetHeight());

                    // PP descriptor set layouts: set 0 (material), set 1 (pass), set 2 (src texture)
                    array_list<gfx::GFXDescriptorSetLayout_sp> descriptorSetLayouts;

                    // set 0: per-material (始终存在)
                    auto ppMatDescSetLayout = ppMat->GetDescriptorSetLayout();
                    if (!ppMatDescSetLayout)
                        continue;
                    descriptorSetLayouts.push_back(ppMatDescSetLayout);

                    // set 1: per-pass (camera + world + light merged)
                    descriptorSetLayouts.push_back(world->GetPerPassResources().GetDescriptorSetLayout());

                    // set 2: src texture (as per-renderer slot)
                    struct InitPPDescLayout
                    {
                        InitPPDescLayout()
                        {
                            gfx::GFXDescriptorSetLayoutDesc info[2] {
                                {
                                    gfx::GFXDescriptorType::CombinedImageSampler,
                                    gfx::GFXGpuProgramStageFlags::VertexFragment,
                                    0, 2
                                },
                                {
                                    gfx::GFXDescriptorType::CombinedImageSampler,
                                    gfx::GFXGpuProgramStageFlags::VertexFragment,
                                    1, 2
                                }
                            };
                            layout = Application::GetGfxApp()->CreateDescriptorSetLayout(info, 2);
                        }
                        gfx::GFXDescriptorSetLayout_sp layout;
                    } _InitPPDescLayout{};

                    descriptorSetLayouts.push_back(_InitPPDescLayout.layout);

                    // 获取 PP pass binding
                    const auto& ppPassBinding = ppMat->GetPassBinding("PostProcess", "");
                    auto ppProgram = ppPassBinding.GetCurrentProgram();
                    if (!ppProgram)
                        continue;

                    auto& ppGpuPrograms = ppProgram->GetGpuPrograms();
                    auto ppShaderConfig = ppMat->GetShader()->GetConfig();

                    gfx::GFXGraphicsPipelineStateParams ppPsoParams{};
                    if (ppShaderConfig && ppShaderConfig->Passes && ppShaderConfig->Passes->size() > 0)
                    {
                        auto& ppPassConfig = (*ppShaderConfig->Passes)[0];
                        if (ppPassConfig->GraphicsPipeline)
                        {
                            auto& gp = ppPassConfig->GraphicsPipeline;
                            ppPsoParams.CullMode = gp->CullMode;
                            ppPsoParams.DepthCompareOp = gp->ZTestOp;
                            ppPsoParams.DepthWriteEnable = gp->ZWriteEnabled;
                            ppPsoParams.DepthTestEnable = true;
                            ppPsoParams.StencilTestEnable = gp->Stencil_Enabled;
                        }
                    }

                    auto pso = pipelineMgr->GetGraphicsPipeline(
                        ppGpuPrograms,
                        ppPsoParams,
                        descriptorSetLayouts,
                        destRt->GetGfxFrameBufferObject()->GetRenderTargetDesc(), {});

                    cmdBuffer.CmdBindGraphicsPipeline(pso.get());

                    {
                        array_list<gfx::GFXDescriptorSet*> descriptorSets;

                        // set 0: per-material (始终存在)
                        descriptorSets.push_back(ppMat->GetDescriptorSet().get());

                        // set 1: per-pass (camera + world + light merged)
                        descriptorSets.push_back(world->GetPerPassResources().GetDescriptorSet().get());

                        // set 2: src texture
                        descriptorSets.push_back(srcResourceDescSet.get());

                        cmdBuffer.CmdBindDescriptorSets(descriptorSets, pso.get());
                    }

                    cmdBuffer.CmdDraw(3);

                    cmdBuffer.CmdEndRenderPass();
                    cmdBuffer.SetFrameBuffer(nullptr);

                    ++ppCount;
                } // end for pp
                if (ppCount)
                {
                    //blit to target
                    cmdBuffer.CmdBlit(lastPPRt->GetGfxRenderTarget0().get(), cam->GetRenderTexture()->GetGfxRenderTarget0().get());
                }
            }

            cmdBuffer.CmdPopDebugInfo();
        }

    }
    void EngineRenderPipeline::AddWorld(World* world)
    {
        m_worlds.push_back(world);
    }
    void EngineRenderPipeline::RemoveWorld(World* world)
    {
        auto it = std::ranges::find(m_worlds, world);
        if (it != m_worlds.end())
        {
            m_worlds.erase(it);
        }
    }

    const char* EngineAppInstance::AppType()
    {
        return "engine";
    }

    static bool _RequestQuit()
    {
        // 请求关闭程序
        return Application::inst()->RequestQuitEvents.IsValidReturnInvoke();
    }

    static void _quitting()
    {
        Logger::Log("engine application is quitting");

        // 通知程序即将关闭
        Application::inst()->QuittingEvents.Invoke();
    }

    void EngineAppInstance::OnPreInitialize(gfx::GFXGlobalConfig* cfg)
    {
        cfg->EnableValid = true;
        cfg->WindowWidth = 1280;
        cfg->WindowHeight = 720;
        strcpy(cfg->Title, "Pulsar v0.1 - vulkan 1.3");
        strcpy(cfg->ProgramName, "Pulsar");
    }

    void EngineAppInstance::OnInitialized()
    {
        Logger::Log("application initialize");
        // SystemInterface::InitializeWindow(title, (int)size.x, (int)size.y);
        // SystemInterface::SetRequestQuitCallBack(_RequestQuit);
        // SystemInterface::SetQuitCallBack(_quitting);
        // RenderInterface::SetViewport(0, 0, (int)size.x, (int)size.y);

        World::Reset<World>("MainWorld");
        Application::GetGfxApp()->SetRenderPipeline(new EngineRenderPipeline{World::Current()});
    }

    void EngineAppInstance::OnTerminate()
    {
        ShaderInstanceCache::Instance().Clear();
        World::Reset(nullptr);
    }

    void EngineAppInstance::OnBeginRender(float dt)
    {
        auto bgc = Color4f{0.2f, 0.2f, 0.2f, 0.2};
        // RenderInterface::Clear(bgc.r, bgc.g, bgc.b, bgc.a);

        World::Current()->Tick(dt);

        static int a = 0;
        a++;
        if (a == 1)
        {
        }

        // RenderInterface::Render();
        // SystemInterface::PollEvents();
    }
    void EngineAppInstance::OnEndRender(float d4)
    {
    }

    bool EngineAppInstance::IsQuit()
    {
        return false;
        // return SystemInterface::GetIsQuit();
    }
    void EngineAppInstance::RequestQuit()
    {
        // SystemInterface::RequestQuitEvents();
    }

    Vector2f EngineAppInstance::GetOutputScreenSize()
    {
        float x, y;
        // detail::RenderInterface::GetDefaultBufferViewport(&x, &y);
        return Vector2f(x, y);
    }
    void EngineAppInstance::SetOutputScreenSize(Vector2f size)
    {
        // detail::RenderInterface::SetViewport(0, 0, (int)size.x, (int)size.y);
    }

    string EngineAppInstance::GetTitle()
    {
        return string();
    }
    void EngineAppInstance::SetTitle(string_view title)
    {
    }
    std::filesystem::path EngineAppInstance::AppRootDir()
    {
        return StringUtil::StringCast(std::filesystem::current_path().generic_u8string());
    }

    AssetManager* EngineAppInstance::GetAssetManager()
    {
        return nullptr;
    }


}