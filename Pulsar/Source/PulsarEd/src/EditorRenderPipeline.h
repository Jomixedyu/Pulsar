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

        virtual void OnRender(gfx::GFXRenderContext* context, const std::vector<gfx::GFXFrameBufferObject*>& framebuffers)
        {
            m_engineRenderPipeline->OnRender(context, {});

            // render editor ui
            auto fbo = static_cast<gfx::GFXFrameBufferObject*>(framebuffers[0]);
            auto& buffer = context->AddCommandBuffer();
            buffer.Begin();

            buffer.SetFrameBuffer(fbo);
            for (auto& rt : fbo->GetRenderTargets())
            {
                buffer.CmdClearColor(rt, 0.0, 0.0, 0.0, 1);
            }
            buffer.CmdBeginFrameBuffer();
            buffer.CmdSetViewport(0, 0, (float)fbo->GetWidth(), (float)fbo->GetHeight());
            ImGuiObject->Render(&buffer);
            buffer.CmdEndFrameBuffer();
            buffer.SetFrameBuffer(nullptr);

            buffer.End();
        }
    };
}