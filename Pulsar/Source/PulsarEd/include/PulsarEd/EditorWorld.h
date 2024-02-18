#pragma once

#include <Pulsar/World.h>
#include "Assembly.h"
#include <Pulsar/Components/CameraComponent.h>
#include <Pulsar/Node.h>

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
    private:
        Node_ref m_camNode;
        Node_ref m_camCtrlNode;
        CameraComponent_ref m_cam;
    };
}

