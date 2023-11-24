#pragma once
#include <PulsarEd/Assembly.h>

namespace pulsared
{
    namespace PImGui
    {
        extern Vector2i GetContentSize();
    }

    class ViewportFrame
    {
    public:
        void SetWorld(World* world) { m_world = world; }
        World* GetWorld() const { return m_world; }
        virtual void Render(float dt);
        virtual void Initialize();
        virtual void Terminate();
    protected:
        World* m_world;
        Vector2i m_viewportSize;
        gfx::GFXDescriptorSetLayout_sp m_descriptorLayout;
        gfx::GFXDescriptorSet_sp m_descriptorSet;
    };

    class SceneEditorViewportFrame : public ViewportFrame
    {
    public:

    };
}