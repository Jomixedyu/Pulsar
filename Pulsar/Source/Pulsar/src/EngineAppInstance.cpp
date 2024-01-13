#include "EngineAppInstance.h"
#include "Assets/StaticMesh.h"
#include "Components/CameraComponent.h"
#include "Components/StaticMeshRendererComponent.h"
#include "Rendering/RenderObject.h"
#include "Scene.h"
#include <Pulsar/Application.h>
#include <Pulsar/EngineAppInstance.h>
#include <Pulsar/ImGuiImpl.h>
#include <Pulsar/Logger.h>
#include <Pulsar/Private/InputInterface.h>
#include <Pulsar/World.h>
#include <filesystem>

namespace pulsar
{

    class RenderTexturePool
    {
    public:
        gfx::GFXFrameBufferObject_sp Get()
        {
            // Application::GetGfxApp()->CreateFrameBufferObject()
        }
    };
    EngineRenderPipeline::EngineRenderPipeline(const std::initializer_list<World*>& worlds)
    {
        for (auto world : worlds)
        {
            AddWorld(world);
        }
    }
    struct RenderData
    {
        gfx::GFXFrameBufferObject* SourceFBO;
        gfx::GFXFrameBufferObject* TargetFBO;
    };
    void EngineRenderPipeline::OnRender(
        gfx::GFXRenderContext* context, gfx::GFXFrameBufferObject* backbuffer)
    {
        auto& cmdBuffer = context->GetCommandBuffer(0);

        for (auto world : m_worlds)
        {
            auto& renderObjects = world->GetRenderObjects();
            auto pipelineMgr = context->GetApplication()->GetGraphicsPipelineManager();

            for (auto cam : world->GetCameraManager().GetCameras())
            {
                auto targetFBO = cam->GetRenderTexture()->GetGfxFrameBufferObject().get();

                cmdBuffer.SetFrameBuffer(targetFBO);

                for (auto& rt : targetFBO->GetRenderTargets())
                {
                    cmdBuffer.CmdClearColor(rt);
                }

                cmdBuffer.CmdBeginFrameBuffer();
                cmdBuffer.CmdSetViewport(0, 0, (float)targetFBO->GetWidth(), (float)targetFBO->GetHeight());

                // combine batch
                std::unordered_map<size_t, rendering::MeshBatch> batchs;
                for (const rendering::RenderObject_sp& renderObject : renderObjects)
                {
                    for (auto& batch : renderObject->GetMeshBatchs())
                    {
                        auto stateHash = batch.GetRenderState();
                        if (batchs.contains(stateHash))
                        {
                            batchs[stateHash].Append(batch);
                        }
                        else
                        {
                            batchs[stateHash] = batch;
                        }
                    }
                }

                // batch render
                for (auto& [state, batch] : batchs)
                {
                    auto shaderPass = batch.Material->GetGfxShaderPass();

                    // bind render state
                    array_list<gfx::GFXDescriptorSetLayout_sp> descriptorSetLayouts;

                    for (auto& refData : targetFBO->RefData)
                    {
                        descriptorSetLayouts.push_back(refData.lock()->GetDescriptorSetLayout());
                    }
                    descriptorSetLayouts.push_back(world->GetWorldDescriptorSet()->GetDescriptorSetLayout());
                    descriptorSetLayouts.push_back(batch.DescriptorSetLayout);
                    if (batch.Material->GetGfxDescriptorSet()->GetDescriptorCount() != 0)
                    {
                        descriptorSetLayouts.push_back(batch.Material->GetGfxDescriptorSetLayout());
                    }

                    auto gfxPipeline = pipelineMgr->GetGraphicsPipeline(shaderPass, descriptorSetLayouts, targetFBO->GetRenderPassLayout(), batch.State);
                    cmdBuffer.CmdBindGraphicsPipeline(gfxPipeline.get());
                    cmdBuffer.CmdSetCullMode(batch.GetCullMode());

                    for (auto& element : batch.Elements)
                    {
                        // bind descriptor sets
                        {
                            array_list<gfx::GFXDescriptorSet*> descriptorSets;
                            // setup cam
                            for (auto& refData : targetFBO->RefData)
                            {
                                descriptorSets.push_back(refData.lock().get());
                            }
                            // setup world
                            descriptorSets.push_back(world->GetWorldDescriptorSet().get());
                            // setup model
                            descriptorSets.push_back(element.ModelDescriptor.get());
                            // setup matinst
                            const auto materialDesc = batch.Material->GetGfxDescriptorSet().get();
                            if(materialDesc->GetDescriptorCount() != 0)
                            {
                                descriptorSets.push_back(materialDesc);
                            }
                            cmdBuffer.CmdBindDescriptorSets(descriptorSets, gfxPipeline.get());
                        }

                        cmdBuffer.CmdBindVertexBuffers({element.Vertex.get()});
                        if (batch.IsUsedIndices)
                        {
                            cmdBuffer.CmdBindIndexBuffer(element.Indices.get());
                        }
                        if (batch.IsUsedIndices)
                        {
                            cmdBuffer.CmdDrawIndexed(element.Indices->GetElementCount());
                        }
                        else
                        {
                            cmdBuffer.CmdDraw(element.Vertex->GetElementCount());
                        }
                    }

                } // end batchs

                cmdBuffer.CmdEndFrameBuffer();
                cmdBuffer.SetFrameBuffer(nullptr);

                // post processing
                RenderTexture_ref lastPPRt;
                auto ppcount = cam->m_postProcessMaterials->size();
                for (size_t i = 0; i < ppcount; ++i)
                {
                    auto ppMat = cam->m_postProcessMaterials->at(i);
                    RenderTexture_ref srcRt;
                    RenderTexture_ref destRt;
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
                    cmdBuffer.CmdBeginFrameBuffer();
                    cmdBuffer.CmdSetViewport(0, 0, (float)targetFBO->GetWidth(), (float)targetFBO->GetHeight());

                    array_list<gfx::GFXDescriptorSetLayout_sp> descriptorSetLayouts;
                    for (auto& refData : targetFBO->RefData)
                    {
                        descriptorSetLayouts.push_back(refData.lock()->GetDescriptorSetLayout());
                    }
                    descriptorSetLayouts.push_back(world->GetWorldDescriptorSet()->GetDescriptorSetLayout());

                    struct InitPPDescLayout
                    {
                        InitPPDescLayout()
                        {
                            gfx::GFXDescriptorSetLayoutInfo info[2] {
                                {
                                    gfx::GFXDescriptorType::CombinedImageSampler,
                                    gfx::GFXShaderStageFlags::VertexFragment,
                                    0, 2
                                },
                                {
                                    gfx::GFXDescriptorType::CombinedImageSampler,
                                    gfx::GFXShaderStageFlags::VertexFragment,
                                    1, 2
                                }
                            };
                            layout = Application::GetGfxApp()->CreateDescriptorSetLayout(info, 2);
                        }
                        gfx::GFXDescriptorSetLayout_sp layout;
                    } _InitPPDescLayout{};

                    descriptorSetLayouts.push_back(_InitPPDescLayout.layout);
                    descriptorSetLayouts.push_back(ppMat->GetGfxDescriptorSetLayout());

                    auto pso = pipelineMgr->GetGraphicsPipeline(
                        ppMat->GetGfxShaderPass(),
                        descriptorSetLayouts,
                        destRt->GetGfxFrameBufferObject()->GetRenderPassLayout(), {});

                    cmdBuffer.CmdBindGraphicsPipeline(pso.get());

                    {
                        array_list<gfx::GFXDescriptorSet*> descriptorSets;
                        // setup cam
                        for (auto& refData : targetFBO->RefData)
                        {
                            descriptorSets.push_back(refData.lock().get());
                        }
                        // setup world
                        descriptorSets.push_back(world->GetWorldDescriptorSet().get());
                        // setup model
                        descriptorSets.push_back(srcResourceDescSet.get());
                        // setup matinst
                        const auto materialDesc = ppMat->GetGfxDescriptorSet().get();
                        if(materialDesc->GetDescriptorCount() != 0)
                        {
                            descriptorSets.push_back(materialDesc);
                        }
                        cmdBuffer.CmdBindDescriptorSets(descriptorSets, pso.get());
                    }

                    cmdBuffer.CmdDraw(3);

                    cmdBuffer.CmdEndFrameBuffer();
                    cmdBuffer.SetFrameBuffer(nullptr);
                }
                if (ppcount)
                {
                    //blit to target
                    cmdBuffer.CmdBlit(lastPPRt->GetGfxRenderTarget0().get(), cam->GetRenderTexture()->GetGfxRenderTarget0().get());
                }
            }
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

    rendering::Pipeline* EngineAppInstance::GetPipeline()
    {
        return nullptr;
    }





}