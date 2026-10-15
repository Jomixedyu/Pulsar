#pragma once
#include "Assembly.h"
#include "Pulsar/Components/CameraComponent.h"

#include <Pulsar/AppInstance.h>
#include <Pulsar/EngineAppInstance.h>
#include <Pulsar/ImGuiImpl.h>
#include <Pulsar/Rendering/RenderScene.h>
#include <Pulsar/Rendering/SceneViewProxy.h>

namespace pulsared
{
    class EditorRenderPipeline final : public EngineRenderPipeline
    {
        using base = EngineRenderPipeline;
    public:
        using base::base;

        std::shared_ptr<ImGuiObject> ImGuiObject;

        // ImGui draw data deep-copied on the main thread (PrepareDrawData) and consumed
        // here on the render thread, so the next NewFrame can overwrite the context.
        ImGuiDrawDataSnapshot ImGuiDrawSnapshot;

        void OnRecord(gfx::GFXCommandBuffer& cmd, gfx::GFXFrameBufferObject* backbuffer) override
        {
            base::OnRecord(cmd, backbuffer);

            // render editor ui

            // Transition every view's render target to shader-read for the UI pass.
            // Views are render-thread proxies (unlike the game-side camera manager).
            for (auto world : m_worlds)
            {
                if (auto* scene = world->GetRenderScene())
                {
                    for (auto& view : scene->GetViews())
                    {
                        if (!view || !view->Data.RenderTarget.IsValid())
                            continue;
                        if (auto colorView = view->Data.RenderTarget.GetColorTextureView())
                            cmd.CmdImageTransitionBarrier(colorView.get(), gfx::GFXResourceLayout::ShaderReadOnly);
                    }
                }
            }

            cmd.SetFrameBuffer(backbuffer);
            for (auto& rt : backbuffer->GetRenderTargets())
            {
                cmd.CmdClearColor(rt->GetTexture(), 0.0, 0.0, 0.0, 1);
            }

            cmd.CmdBeginRenderPass("Editor UI");
            cmd.CmdSetViewport(0, 0, (float)backbuffer->GetWidth(), (float)backbuffer->GetHeight());
            ImGuiObject->Render(&cmd, ImGuiDrawSnapshot);
            cmd.CmdEndRenderPass();
            cmd.SetFrameBuffer(nullptr);
        }
    };
}
