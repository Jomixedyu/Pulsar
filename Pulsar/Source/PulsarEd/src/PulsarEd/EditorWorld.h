#pragma once

#include "Assembly.h"
#include <Pulsar/SelectionSet.h>
#include <Pulsar/Components/CameraComponent.h>
#include <Pulsar/Node.h>
#include <Pulsar/World.h>
#include <Pulsar/Assets/NodeCollection.h>
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

        static EditorWorld* DuplicateAndBeginPlay();
        static void EndPlayAndRestore();

    public:
        using World::World;

        virtual void Tick(float dt) override;

        void AddDirectionalLight();
        virtual CameraComponent_ref GetCurrentCamera() override;
    public:
        virtual void OnWorldBegin() override;
        virtual void OnWorldEnd() override;
    protected:
        virtual void OnLoadingResidentScene(RCPtr<NodeCollection> scene) override;
        virtual void OnUnloadingResidentScene(RCPtr<NodeCollection> scene) override;
        virtual void OnSceneLoading(RCPtr<NodeCollection> scene) override;
        virtual void OnSceneUnloading(RCPtr<NodeCollection> scene) override;
        virtual const char* GetWorldTypeName() const { return StaticWorldTypeName(); }
        static const char* StaticWorldTypeName() { return "EditorWorld"; }
    public:
        EdTool* GetTool() const { return m_tool.get(); }
        void SetTool(std::unique_ptr<EdTool>&& tool);

        int GetGizmoSpace() const { return m_gizmoSpace; }
        void SetGizmoSpace(int space) { m_gizmoSpace = space; }
    private:
        ObjectPtr<Node> m_camNode;
        CameraComponent_ref m_cam;


        std::unique_ptr<EdTool> m_tool = nullptr;
        int m_gizmoSpace = 0; // 0 = Local (ImGuizmo::LOCAL), 1 = World (ImGuizmo::WORLD)
    };
}

