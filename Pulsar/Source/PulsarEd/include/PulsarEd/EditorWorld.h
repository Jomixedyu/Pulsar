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

        static EditorWorld* DuplicateAndBeginPlay();
        static void EndPlayAndRestore();

    public:
        using World::World;

        virtual void Tick(float dt) override;

        void AddDirectionalLight();
        virtual CameraComponent_ref GetCurrentCamera() override;
        bool IsSelectedNode(const ObjectPtr<Node>& node) const override;
    public:
        virtual void OnWorldBegin() override;
        virtual void OnWorldEnd() override;
    protected:
        virtual void OnLoadingResidentScene(RCPtr<Scene> scene) override;
        virtual void OnUnloadingResidentScene(RCPtr<Scene> scene) override;
        virtual void OnSceneLoading(RCPtr<Scene> scene) override;
        virtual void OnSceneUnloading(RCPtr<Scene> scene) override;
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

