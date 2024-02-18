#pragma once
#include <PulsarEd/Assembly.h>

namespace pulsared
{
    namespace PImGui
    {
        extern Vector2f GetContentSize();
    }

    class ViewportFrame
    {
    public:
        virtual ~ViewportFrame() = default;
        void SetWorld(World* world);
        World* GetWorld() const { return m_world; }
        virtual void Render(float dt);
        virtual void Initialize();
        virtual void Terminate();
    protected:
        World* m_world = nullptr;
        bool m_newWorld {};
        Vector2f m_viewportSize{};
        gfx::GFXDescriptorSetLayout_sp m_descriptorLayout;
        gfx::GFXDescriptorSet_sp m_descriptorSet;

        bool m_altPressed = false;
        bool m_leftMousePressed = false;
        bool m_middleMousePressed = false;
        bool m_rightMousePressed = false;
        Vector2f m_latestMousePos{};
    public:
        float m_scaleSpeed = 1.f;
        bool m_enabledMove{true};
        bool m_enabledRotate{true};
    };

    class SceneEditorViewportFrame : public ViewportFrame
    {
    public:

    };
}