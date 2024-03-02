#pragma once
#include "EdTools/EdTool.h"

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

        EdTool* GetTool() const { return m_tool.get(); }
        void SetTool(std::unique_ptr<EdTool>&& tool);
    protected:
        World* m_world = nullptr;
        bool m_newWorld {};
        Vector2f m_viewportSize{};
        gfx::GFXDescriptorSetLayout_sp m_descriptorLayout;
        gfx::GFXDescriptorSet_sp m_descriptorSet;
        std::unique_ptr<EdTool> m_tool = nullptr;
    };

    class SceneEditorViewportFrame : public ViewportFrame
    {
    public:

    };
}