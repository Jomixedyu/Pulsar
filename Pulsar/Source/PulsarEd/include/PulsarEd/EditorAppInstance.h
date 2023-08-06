#pragma once
#include "Assembly.h"
#include <Pulsar/AppInstance.h>
#include <Pulsar/BuiltinRP.h>
#include <Pulsar/ImGuiImpl.h>
#include <Pulsar/EngineAppInstance.h>

namespace pulsared
{

    class EditorRenderPipeline : public gfx::GFXRenderPipeline
    {
        EngineRenderPipeline* m_engineRenderPipeline;
    public:
        EditorRenderPipeline(World* world)
        {
            m_engineRenderPipeline = new EngineRenderPipeline(world);
        }
        ~EditorRenderPipeline()
        {
            delete m_engineRenderPipeline;
            m_engineRenderPipeline = nullptr;
        }

        std::shared_ptr<ImGuiObject> ImGuiObject;
        std::vector<gfx::GFXFrameBufferObject*> m_frameBuffers;

        virtual void OnRender(gfx::GFXRenderContext* context, const std::vector<gfx::GFXFrameBufferObject*>& renderTargets)
        {
            //m_engineRenderPipeline->OnRender(context, m_frameBuffers);

            // view port
            auto rt = static_cast<gfx::GFXFrameBufferObject*>(renderTargets[0]);
            auto& buffer = context->AddCommandBuffer();
            buffer.Begin();

            buffer.SetFrameBuffer(rt);
            buffer.CmdClearColor(0.0, 0.0, 0.0, 1);
            buffer.CmdBeginFrameBuffer();
            buffer.CmdSetViewport(0, 0, rt->GetWidth(), rt->GetHeight());
            ImGuiObject->Render(&buffer);
            buffer.CmdEndFrameBuffer();
            buffer.SetFrameBuffer(nullptr);

            buffer.End();
        }
    };


    class EditorAppInstance : public AppInstance
    {

    public:
        virtual const char* AppType() override;
        virtual void RequestQuit() override;
        virtual Vector2f GetOutputScreenSize() override;
        virtual void SetOutputScreenSize(Vector2f size) override;
        virtual string GetTitle() override;
        virtual void SetTitle(string_view title) override;
        virtual string AppRootDir() override;
        virtual void OnPreInitialize(gfx::GFXGlobalConfig* config) override;
        virtual void OnInitialized() override;
        virtual void OnTerminate() override;
        virtual void OnBeginRender(float dt) override;
        virtual void OnEndRender(float dt) override;
        virtual bool IsQuit() override;
        virtual rendering::Pipeline* GetPipeline() override;

        virtual Vector2f GetAppSize();
        virtual void SetAppSize(Vector2f size);

        virtual bool IsInteractiveRendering() const;

        void StartInteractiveRendering();
        void StopInteractiveRendering();

    protected:
        std::shared_ptr<ImGuiObject> m_gui = nullptr;

        Vector2f output_size_;

        bool m_isPlaying = false;
    };

    inline EditorAppInstance* GetEdApp()
    {
        return static_cast<EditorAppInstance*>(Application::inst());
    }
}