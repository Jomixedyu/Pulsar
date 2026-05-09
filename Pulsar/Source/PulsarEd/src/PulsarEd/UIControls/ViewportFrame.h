#pragma once
#include "EdTools/EdTool.h"

#include <PulsarEd/Assembly.h>

namespace pulsared
{

    class ViewportFrame
    {
    public:
        virtual ~ViewportFrame() = default;
        void SetWorld(World* world);
        World* GetWorld() const { return m_world; }
        virtual void Render(float dt);
        virtual void Initialize();
        virtual void Terminate();

        void SetIsPreviewCamera(bool value) { m_isPreviewCam = value; }
        bool GetIsPreviewCamera() const { return m_isPreviewCam; }

        void SetEnableEdToolTick(bool value) { m_enableEdToolTick = value; }
        bool GetEnableEdToolTick() const { return m_enableEdToolTick; }

    protected:
        World* m_world = nullptr;
        bool m_newWorld {};
        bool m_isPreviewCam = true;
        bool m_enableEdToolTick = true;
        Vector2f m_viewportSize{};
        gfx::GFXDescriptorSetLayout_sp m_descriptorLayout;
        gfx::GFXDescriptorSet_sp m_descriptorSet;
    };

    class SceneEditorViewportFrame : public ViewportFrame
    {
    public:

    };
}