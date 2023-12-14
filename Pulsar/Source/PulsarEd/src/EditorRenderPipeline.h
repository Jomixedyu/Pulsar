#pragma once
#include "Assembly.h"
#include <Pulsar/AppInstance.h>
#include <Pulsar/ImGuiImpl.h>
#include <Pulsar/EngineAppInstance.h>

namespace pulsared
{
    class EditorRenderPipeline final : public EngineRenderPipeline
    {
        using base = EngineRenderPipeline;
    public:
        using base::base;

        std::shared_ptr<ImGuiObject> ImGuiObject;

        void OnRender(gfx::GFXRenderContext* context, gfx::GFXFrameBufferObject* backbuffer) override
        {
            base::OnRender(context, backbuffer);

            // render editor ui
            auto& buffer = context->AddCommandBuffer();
            buffer.Begin();

            buffer.SetFrameBuffer(backbuffer);
            for (auto& rt : backbuffer->GetRenderTargets())
            {
                buffer.CmdClearColor(rt, 0.0, 0.0, 0.0, 1);
            }
            buffer.CmdBeginFrameBuffer();
            buffer.CmdSetViewport(0, 0, (float)backbuffer->GetWidth(), (float)backbuffer->GetHeight());
            ImGuiObject->Render(&buffer);
            buffer.CmdEndFrameBuffer();
            buffer.SetFrameBuffer(nullptr);

            buffer.End();
        }
    };
}