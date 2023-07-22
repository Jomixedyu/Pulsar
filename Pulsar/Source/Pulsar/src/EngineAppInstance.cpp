#include "EngineAppInstance.h"
#include <Pulsar/EngineAppInstance.h>
#include <Pulsar/Application.h>
#include <Pulsar/World.h>
#include <Pulsar/Logger.h>
#include <filesystem>
#include <Pulsar/ImGuiImpl.h>
#include <Pulsar/Private/InputInterface.h>

#include "Assets/StaticMesh.h"
#include "Scene.h"

namespace pulsar
{
    static StaticMeshVertexBuildDataArray BuildUVSphere(float radius, int u, int v)
    {
        StaticMeshVertexBuildDataArray mesh;
        
        StaticMeshVertexBuildData point;
        point.Position = { 0,1,0 };
        for (size_t i = 0; i < v - 1; i++)
        {

        }
    }


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
            for (auto& fb : framebuffers)
            {
                auto& buffer = context->AddCommandBuffer();
                buffer.Begin();
                buffer.SetFrameBuffer(fb);

                buffer.CmdClearColor(1, 0, 1, 1);
                buffer.CmdBeginFrameBuffer();
                buffer.CmdSetViewport(0, 0, fb->GetWidth(), fb->GetHeight());

                //array_list< renderObjects = m_world->GetRenderObjects();
                ////batch render
                //for (auto& i : renderObjects)
                //{
                //    
                //}

                //post processing

                buffer.CmdEndFrameBuffer();
                buffer.SetFrameBuffer(nullptr);
                buffer.End();
            }
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