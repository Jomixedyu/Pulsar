#pragma once
#include "EdTools/EdTool.h"

#include <PulsarEd/Assembly.h>

namespace pulsared
{
    class ViewportFrame
    {
    public:
        virtual ~ViewportFrame();
        void SetWorld(World* world);
        World* GetWorld() const { return m_world; }
        virtual void Render(float dt);
        virtual void Initialize();
        virtual void Terminate();

        void SetIsPreviewCamera(bool value) { m_isPreviewCam = value; }
        bool GetIsPreviewCamera() const { return m_isPreviewCam; }

        void SetEnableEdToolTick(bool value) { m_enableEdToolTick = value; }
        bool GetEnableEdToolTick() const { return m_enableEdToolTick; }

        // Last known viewport geometry from the most recent Render() call
        float GetLastViewportX() const { return m_lastViewportX; }
        float GetLastViewportY() const { return m_lastViewportY; }
        float GetLastViewportW() const { return m_lastViewportW; }
        float GetLastViewportH() const { return m_lastViewportH; }
        bool GetLastHasFocus() const { return m_lastHasFocus; }

    protected:
        World* m_world = nullptr;
        bool m_newWorld {};
        bool m_isPreviewCam = true;
        bool m_enableEdToolTick = true;
        Vector2f m_viewportSize{};
        gfx::GFXDescriptorSetLayout_sp m_descriptorLayout;
        gfx::GFXDescriptorSet_sp m_descriptorSet;

        float m_lastViewportX = 0.0f;
        float m_lastViewportY = 0.0f;
        float m_lastViewportW = 0.0f;
        float m_lastViewportH = 0.0f;
        bool m_lastHasFocus = false;
    };

    class SceneEditorViewportFrame : public ViewportFrame
    {
    public:

    };
}