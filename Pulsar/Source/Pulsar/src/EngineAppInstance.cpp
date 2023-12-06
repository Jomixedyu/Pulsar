#include "EngineAppInstance.h"
#include <Pulsar/EngineAppInstance.h>
#include <Pulsar/Application.h>
#include <Pulsar/World.h>
#include <Pulsar/Logger.h>
#include <filesystem>
#include <Pulsar/ImGuiImpl.h>
#include <Pulsar/Private/InputInterface.h>
#include "Rendering/RenderObject.h"
#include "Assets/StaticMesh.h"
#include "Components/StaticMeshRendererComponent.h"
#include "Scene.h"

namespace pulsar
{

    void EngineRenderPipeline::OnRender(gfx::GFXRenderContext* context, const std::vector<gfx::GFXFrameBufferObject*>& framebuffers)
    {
        array_list<gfx::GFXFrameBufferObject*> targetFBOs;
        targetFBOs.insert(targetFBOs.end(), framebuffers.begin(), framebuffers.end());

        // fill cameras rendertarget
        for (auto& cam : m_world->GetCameraManager().GetCameras())
        {
            targetFBOs.push_back(cam->GetRenderTarget()->GetGfxFrameBufferObject().get());
        }

        if (targetFBOs.size() == 0)
        {
            return;
        }
        auto& renderObjects = m_world->GetRenderObjects();
        auto pipelineMgr = context->GetApplication()->GetGraphicsPipelineManager();

        auto& cmdBuffer = context->AddCommandBuffer();
        cmdBuffer.Begin();

        for (auto& targetFBO : targetFBOs)
        {
            cmdBuffer.SetFrameBuffer(targetFBO);

            for (auto& rt : targetFBO->GetRenderTargets())
            {
                cmdBuffer.CmdClearColor(rt);
            }

            cmdBuffer.CmdBeginFrameBuffer();
            cmdBuffer.CmdSetViewport(0, 0, (float)targetFBO->GetWidth(), (float)targetFBO->GetHeight());

            //combine batch
            std::unordered_map<size_t, rendering::MeshBatch> batchs;
            for (const rendering::RenderObject_sp& renderObject : renderObjects)
            {
                for (auto& batch : renderObject->GetMeshBatchs())
                {
                    batchs[batch.GetRenderState()] = batch;
                }
            }

            // batch render
            for (auto& [state, batch] : batchs)
            {
                const auto passCount = batch.Material->GetShaderPassCount();
                for (size_t i = 0; i < passCount; i++)
                {
                    auto shaderPass = batch.Material->GetGfxShaderPass(i);

                    // bind render state
                    array_list<gfx::GFXDescriptorSetLayout_sp> descriptorSetLayouts;

                    for (auto& refData : targetFBO->RefData)
                    {
                        descriptorSetLayouts.push_back(refData.lock()->GetDescriptorSetLayout());
                    }
                    descriptorSetLayouts.push_back(m_world->GetWorldDescriptorSet()->GetDescriptorSetLayout());
                    descriptorSetLayouts.push_back(batch.DescriptorSetLayout);

                    auto gfxPipeline = pipelineMgr->GetGraphicsPipeline(shaderPass, descriptorSetLayouts, targetFBO->GetRenderPassLayout());
                    cmdBuffer.CmdBindGraphicsPipeline(gfxPipeline.get());

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
                            //setup world
                            descriptorSets.push_back(m_world->GetWorldDescriptorSet().get());
                            //setup model
                            descriptorSets.push_back(element.ModelDescriptor.get());
                            //setup matinst
                            // todo
                            cmdBuffer.CmdBindDescriptorSets(descriptorSets, gfxPipeline.get());
                        }

                        cmdBuffer.CmdBindVertexBuffers({ element.Vertex.get() });
                        if (batch.IsUsedIndices)
                        {
                            cmdBuffer.CmdBindIndexBuffer(element.Indices.get());
                        }
                        if (batch.IsUsedIndices)
                        {
                            cmdBuffer.CmdDrawIndexed(element.Indices->GetSize() / sizeof(uint32_t));
                        }
                        else
                        {
                            cmdBuffer.CmdDraw(element.Vertex->GetSize());
                        }
                    }

                }
            }

            //post processing
            cmdBuffer.CmdEndFrameBuffer();
            cmdBuffer.SetFrameBuffer(nullptr);
        }
        cmdBuffer.End();
    }
    
    const char* EngineAppInstance::AppType()
    {
        return "engine";
    }

    static bool _RequestQuit()
    {
        //请求关闭程序
        return Application::inst()->RequestQuitEvents.IsValidReturnInvoke();
    }

    static void _quitting()
    {
        Logger::Log ("engine application is quitting");

        //通知程序即将关闭
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
        //SystemInterface::InitializeWindow(title, (int)size.x, (int)size.y);
        //SystemInterface::SetRequestQuitCallBack(_RequestQuit);
        //SystemInterface::SetQuitCallBack(_quitting);
        //RenderInterface::SetViewport(0, 0, (int)size.x, (int)size.y);

        World::Reset<World>();
        Application::GetGfxApp()->SetRenderPipeline(new EngineRenderPipeline(World::Current()));
    }

    void EngineAppInstance::OnTerminate()
    {

        World::Reset(nullptr);
    }

    void EngineAppInstance::OnBeginRender(float dt)
    {
        auto bgc = Color4f{ 0.2f, 0.2f ,0.2f, 0.2 };
        //RenderInterface::Clear(bgc.r, bgc.g, bgc.b, bgc.a);

        World::Current()->Tick(dt);

        static int a = 0;
        a++;
        if (a == 1)
        {

        }

        //RenderInterface::Render();
        //SystemInterface::PollEvents();
        
    }
    void EngineAppInstance::OnEndRender(float d4)
    {
    }

    bool EngineAppInstance::IsQuit()
    {
        return false;
        //return SystemInterface::GetIsQuit();
    }
    void EngineAppInstance::RequestQuit()
    {
        //SystemInterface::RequestQuitEvents();
    }

    Vector2f EngineAppInstance::GetOutputScreenSize()
    {
        float x, y;
        //detail::RenderInterface::GetDefaultBufferViewport(&x, &y);
        return Vector2f(x, y);
    }
    void EngineAppInstance::SetOutputScreenSize(Vector2f size)
    {
        //detail::RenderInterface::SetViewport(0, 0, (int)size.x, (int)size.y);
    }

    string EngineAppInstance::GetTitle()
    {
        return string();
    }
    void EngineAppInstance::SetTitle(string_view title)
    {
    }
    string EngineAppInstance::AppRootDir()
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