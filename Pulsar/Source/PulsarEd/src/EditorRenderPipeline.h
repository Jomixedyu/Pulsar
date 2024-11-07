#pragma once
#include "Assembly.h"
#include "Pulsar/Components/CameraComponent.h"

#include <Pulsar/AppInstance.h>
#include <Pulsar/EngineAppInstance.h>
#include <Pulsar/ImGuiImpl.h>

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
            auto& cmd = context->AddCommandBuffer();
            cmd.Begin();
            base::OnRender(context, backbuffer);

            // render editor ui

            for (auto world : m_worlds)
            {
                for (auto cam : world->GetCameraManager().GetCameras())
                {
                    auto rt = cam->GetRenderTexture()->GetGfxRenderTarget0();
                    cmd.CmdImageTransitionBarrier(rt.get(), gfx::GFXResourceLayout::ShaderReadOnly);
                }
            }

            cmd.SetFrameBuffer(backbuffer);
            for (auto& rt : backbuffer->GetRenderTargets())
            {
                cmd.CmdClearColor(rt->GetTexture(), 0.0, 0.0, 0.0, 1);
            }
            cmd.CmdBeginFrameBuffer();
            cmd.CmdSetViewport(0, 0, (float)backbuffer->GetWidth(), (float)backbuffer->GetHeight());
            ImGuiObject->Render(&cmd);
            cmd.CmdEndFrameBuffer();
            cmd.SetFrameBuffer(nullptr);

            cmd.End();
        }
    };
}