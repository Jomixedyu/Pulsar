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
        using World::World;

        virtual void Tick(float dt) override;

        void AddGrid3d();
        virtual CameraComponent_ref GetPreviewCamera() override;
    protected:
        virtual void OnWorldBegin() override; // NOLINT(*-use-override)
        virtual void OnWorldEnd() override;
        virtual void OnLoadingPersistentScene(ObjectPtr<Scene> scene) override;
        virtual void OnUnloadingPersistentScene(ObjectPtr<Scene> scene) override;
        virtual void OnSceneLoading(ObjectPtr<Scene> scene) override;
        virtual void OnSceneUnloading(ObjectPtr<Scene> scene) override;
    private:
        Node_ref m_camNode;
        Node_ref m_camCtrlNode;
        CameraComponent_ref m_cam;
    };
}

