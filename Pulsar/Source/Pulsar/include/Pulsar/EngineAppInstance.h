#pragma once
#include "AppInstance.h"


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
            if (framebuffers.size() == 0)
            {
                return;
            }
            auto& renderObjects = m_world->GetRenderObjects();
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
                std::unordered_map<Material_sp, rendering::MeshBatch> batchs;
                //for (rendering::RenderObject* renderObject : renderObjects)
                //{
                //    for (auto& batch : renderObject->GetMeshBatchs())
                //    {
                //        batchs[batch.Material].Append(batch);
                //    }
                //}

                //for (auto& [handle, batch] : batchs)
                //{

                    //auto pipeline = pipelineMgr->GetGraphicsPipeline();
                    //cmd.CmdBindGraphicsPipeline(pipeline.get());

                    //for (size_t i = 0; i < batch.Vertex.size(); i++)
                    //{
                        //cmd.CmdBindVertexBuffers();
                        //cmd.CmdBindIndexBuffer();
                        //cmd.CmdBindDescriptorSets();
                        //cmd.CmdDrawIndexed(batch.Indices[i]->GetSize());
                //    }

                //}

                //post processing

                cmd.CmdEndFrameBuffer();
                cmd.SetFrameBuffer(nullptr);
            }
            cmd.End();
        }

    };

    class EngineAppInstance : public AppInstance
    {
    public:
        virtual const char* AppType() override;
        virtual void OnInitialized() override;
        virtual void OnTerminate() override;
        virtual void OnBeginRender(float dt) override;
        virtual bool IsQuit() override;
        virtual void RequestQuit() override;
        virtual Vector2f GetOutputScreenSize() override;
        virtual void SetOutputScreenSize(Vector2f size) override;
        virtual string GetTitle() override;
        virtual void SetTitle(string_view title) override;
        virtual string AppRootDir() override;

        virtual rendering::Pipeline* GetPipeline() override;

        virtual void OnPreInitialize(gfx::GFXGlobalConfig* cfg) override;

        virtual void OnEndRender(float d4) override;

    };

}