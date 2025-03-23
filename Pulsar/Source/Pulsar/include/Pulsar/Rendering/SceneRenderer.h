#pragma once
#include "RenderingScene.h"
#include "rdg/RDGBuilder.h"

namespace pulsar::rendering
{

    class SceneRenderer
    {
    public:
        SceneRenderer()
        {

        }
        virtual ~SceneRenderer() = default;

        virtual void Render(rdg::RDGBuilder& builder) = 0;
        virtual void GatherMeshes() = 0;

        gfx::GFXBuffer_sp m_uniform;
        RenderingScene* m_scene = nullptr;
    };

    class ForwardRenderer : public SceneRenderer
    {
    public:
        void Render(rdg::RDGBuilder& builder) override;
    };
}