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
        virtual void Tick(float dt) override;


        virtual CameraComponent_ref GetPreviewCamera() override;
    protected:
        virtual void OnWorldBegin() override;
        virtual void OnWorldEnd() override;
        virtual void OnLoadingPresistentScene(ObjectPtr<Scene> scene);
        virtual void OnUnloadingPresistentScene(ObjectPtr<Scene> scene);
        virtual void OnSceneLoading(ObjectPtr<Scene> scene) override;
        virtual void OnSceneUnloading(ObjectPtr<Scene> scene) override;
    private:
        Node_ref m_camNode;
        Node_ref m_camCtrlNode;
        CameraComponent_ref m_cam;
    };
}

