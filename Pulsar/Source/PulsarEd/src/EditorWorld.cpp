#include "EditorWorld.h"
#include <PulsarEd/Components/StdEditCameraControllerComponent.h>
#include <PulsarEd/Components/Grid3DComponent.h>
#include <Pulsar/Scene.h>

namespace pulsared
{
    void EditorWorld::Tick(float dt)
    {



        base::Tick(dt);

    }

    CameraComponent_ref EditorWorld::GetPreviewCamera()
    {
        return base::GetPreviewCamera();
    }

    void EditorWorld::OnWorldBegin()
    {
        base::OnWorldBegin();
    }
    void EditorWorld::OnWorldEnd()
    {
        base::OnWorldEnd();
    }

    void EditorWorld::OnLoadingPresistentScene(ObjectPtr<Scene> scene)
    {
        base::OnLoadingPresistentScene(scene);

        auto camCtrlNode = Node::StaticCreate("EdCameraController", nullptr, OF_NoPack);
        auto camNode = Node::StaticCreate("EdCamera", camCtrlNode->GetTransform(), OF_NoPack);
        this->m_camNode = camNode;
        this->m_camCtrlNode = camCtrlNode;

        m_cam = camNode->AddComponent<CameraComponent>();

        auto camRt = RenderTexture::StaticCreate(2, 2, true, true);
        //GetDeferredDestroyedQueue().push_back(camRt);

        m_cam->SetRenderTarget(camRt);
        
        m_cam->cameraMode = CameraMode::Perspective;
        m_cam->SetBackgroundColor(Color4f{ 0.3f, 0.3f, 0.3f, 1.0f });
        m_cam->fov = 45.f;
        m_cam->near = 0.01f;
        m_cam->far = 10000.f;
        m_cam->size_ = { 1280.f, 720.f };
        camNode->GetTransform()->SetPosition({ 0.f, 7.f, 30.f });

        camCtrlNode->AddComponent<StdEditCameraControllerComponent>();

        scene->AddNode(camCtrlNode);
        scene->AddNode(camNode);

        auto grid3d = Node::StaticCreate("Grid3d", nullptr, OF_NoPack);
        grid3d->AddComponent<Grid3DComponent>();
        scene->AddNode(grid3d);
    }
    void EditorWorld::OnUnloadingPresistentScene(ObjectPtr<Scene> scene)
    {
        base::OnUnloadingPresistentScene(scene);
        DestroyObject(m_cam->GetRenderTarget());
        m_cam->SetRenderTarget(nullptr);
    }

    void EditorWorld::OnSceneLoading(ObjectPtr<Scene> scene)
    {
        base::OnSceneLoading(scene);

    }
    
    void EditorWorld::OnSceneUnloading(ObjectPtr<Scene> scene)
    {
        base::OnSceneUnloading(scene);

    }
}