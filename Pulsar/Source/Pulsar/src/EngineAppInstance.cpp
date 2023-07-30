#include "EngineAppInstance.h"
#include <Pulsar/EngineAppInstance.h>
#include <Pulsar/Application.h>
#include <Pulsar/World.h>
#include <Pulsar/Logger.h>
#include <filesystem>
#include <Pulsar/ImGuiImpl.h>
#include <Pulsar/Private/InputInterface.h>

#include "Assets/StaticMesh.h"
#include "Components/StaticMeshRendererComponent.h"
#include "Scene.h"

namespace pulsar
{


    class EngineRenderPipeline : public gfx::GFXRenderPipeline
    {
    public:
        World* m_world;
        EngineRenderPipeline(World* world)
            : m_world(world)
        {
        }

        virtual void OnRender(gfx::GFXRenderContext* context, const std::vector<gfx::GFXFrameBufferObject*>& framebuffers) override
        {
            array_list<rendering::RenderObject*> renderObjects = m_world->GetRenderObjects();
            auto pipelineMgr = context->GetApplication()->GetGraphicsPipelineManager();

            auto& cmd = context->AddCommandBuffer();
            cmd.Begin();

            for (auto& fb : framebuffers)
            {
                cmd.SetFrameBuffer(fb);

                cmd.CmdClearColor(1, 0, 1, 1);
                cmd.CmdBeginFrameBuffer();
                cmd.CmdSetViewport(0, 0, fb->GetWidth(), fb->GetHeight());

                //batch render
                for (rendering::RenderObject* renderObject : renderObjects)
                {
                    
                    auto pipeline = pipelineMgr->GetGraphicsPipeline(renderObject.);

                    cmd.CmdBindGraphicsPipeline();
                    cmd.CmdBindVertexBuffers();
                    cmd.CmdBindIndexBuffer();
                    cmd.CmdBindDescriptorSets();
                    cmd.CmdDrawIndexed();
                }

                //post processing

                cmd.CmdEndFrameBuffer();
                cmd.SetFrameBuffer(nullptr);
            }
            cmd.End();
            context->Submit();
        }

    };
    
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

        World::Reset(new World);
        Application::GetGfxApp()->SetRenderPipeline(new EngineRenderPipeline(World::Current()));
    }

    void EngineAppInstance::OnTerminate()
    {

        World::Reset(nullptr);
    }

    void EngineAppInstance::OnBeginRender(float dt)
    {
        auto bgc = LinearColorf{ 0.2f, 0.2f ,0.2f, 0.2 };
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
        int32_t x, y;
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

    rendering::Pipeline* EngineAppInstance::GetPipeline()
    {
        return nullptr;
    }





}