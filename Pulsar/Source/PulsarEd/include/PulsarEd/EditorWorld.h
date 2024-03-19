#pragma once

#include "Assembly.h"
#include "SelectionSet.h"
#include <Pulsar/Components/CameraComponent.h>
#include <Pulsar/Node.h>
#include <Pulsar/World.h>
#include "EdTools/EdTool.h"

namespace pulsared
{
    class EditorWorld : public World
    {
        using base = World;
    public:

        static void PushPreviewWorld(std::unique_ptr<World> world);
        static bool PreviewWorldStackEmpty();
        static void PopPreviewWorld();
        static World* GetPreviewWorld();

    public:
        using World::World;

        virtual void Tick(float dt) override;

        void AddGrid3d();
        void AddDirectionalLight();
        virtual CameraComponent_ref GetPreviewCamera() override;
    public:
        virtual void OnWorldBegin() override; // NOLINT(*-use-override)
        virtual void OnWorldEnd() override;
    protected:
        virtual void OnLoadingResidentScene(ObjectPtr<Scene> scene) override;
        virtual void OnUnloadingResidentScene(ObjectPtr<Scene> scene) override;
        virtual void OnSceneLoading(ObjectPtr<Scene> scene) override;
        virtual void OnSceneUnloading(ObjectPtr<Scene> scene) override;
        virtual const char* GetWorldTypeName() const { return StaticWorldTypeName(); }
        static const char* StaticWorldTypeName() { return "EditorWorld"; }
    public:
        SelectionSet<Node>& GetSelection() { return m_selection; }
        const SelectionSet<Node>& GetSelection() const { return m_selection; }
        EdTool* GetTool() const { return m_tool.get(); }
        void SetTool(std::unique_ptr<EdTool>&& tool);
    private:
        Node_ref m_camNode;
        Node_ref m_camCtrlNode;
        CameraComponent_ref m_cam;

        SelectionSet<Node> m_selection;
        std::unique_ptr<EdTool> m_tool = nullptr;
    };
}

