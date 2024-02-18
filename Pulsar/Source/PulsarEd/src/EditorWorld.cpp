#include "EditorWorld.h"

#include "Pulsar/Components/DirectionalLightComponent.h"
#include "Pulsar/EngineAppInstance.h"

#include <Pulsar/Scene.h>
#include <PulsarEd/Components/Grid3DComponent.h>
#include <PulsarEd/Components/StdEditCameraControllerComponent.h>
#include <stack>

namespace pulsared
{
    static std::stack<std::unique_ptr<World>> _worldStack;
    void EditorWorld::PushPreviewWorld(std::unique_ptr<World> world)
    {
        world->OnWorldBegin();
        auto pipeline = static_cast<EngineRenderPipeline*>(Application::GetGfxApp()->GetRenderPipeline());
        pipeline->AddWorld(world.get());
        _worldStack.push(std::move(world));
    }
    bool EditorWorld::PreviewWorldStackEmpty()
    {
        return _worldStack.empty();
    }
    void EditorWorld::PopPreviewWorld()
    {
        auto world = _worldStack.top().get();

        auto pipeline = static_cast<EngineRenderPipeline*>(Application::GetGfxApp()->GetRenderPipeline());
        pipeline->RemoveWorld(world);

        world->OnWorldEnd();
        _worldStack.pop();
    }
    World* EditorWorld::GetPreviewWorld()
    {
        if (!_worldStack.empty())
        {
            return _worldStack.top().get();
        }
        return base::Current();
    }
    void EditorWorld::Tick(float dt)
    {

        base::Tick(dt);
    }
    void EditorWorld::AddGrid3d()
    {
        auto grid3d = GetResidentScene()->NewNode("__ReferenceGrid3d", nullptr, OF_NoPack);
        grid3d->AddComponent<Grid3DComponent>();
    }
    void EditorWorld::AddDirectionalLight()
    {
        auto dlight = GetResidentScene()->NewNode("Directional Light");
        dlight->AddComponent<DirectionalLightComponent>();
        dlight->GetTransform()->TranslateRotateEuler({-3, 3, -3}, {45, 45, 0});
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

    void EditorWorld::OnLoadingResidentScene(ObjectPtr<Scene> scene)
    {
        base::OnLoadingResidentScene(scene);

        auto camCtrlNode = scene->NewNode("EdCameraController", nullptr, OF_NoPack);
        auto camNode = scene->NewNode("EdCamera", camCtrlNode, OF_NoPack);
        camNode->GetTransform()->Translate({0, 0, -2});
        this->m_camNode = camNode;
        this->m_camCtrlNode = camCtrlNode;

        m_cam = camNode->AddComponent<CameraComponent>();

        m_cam->SetProjectionMode(CameraProjectionMode::Perspective);
        m_cam->SetBackgroundColor(Color4f{0.3f, 0.3f, 0.3f, 1.0f});
        m_cam->SetFOV(45.f);
        m_cam->SetNear(0.01f);
        m_cam->SetFar(10000.f);

        camCtrlNode->AddComponent<StdEditCameraControllerComponent>();
    }

    void EditorWorld::OnUnloadingResidentScene(ObjectPtr<Scene> scene)
    {
        base::OnUnloadingResidentScene(scene);
        DestroyObject(m_cam->GetRenderTexture());
        m_cam->SetRenderTexture(nullptr);
    }

    void EditorWorld::OnSceneLoading(ObjectPtr<Scene> scene)
    {
        base::OnSceneLoading(scene);
    }

    void EditorWorld::OnSceneUnloading(ObjectPtr<Scene> scene)
    {
        base::OnSceneUnloading(scene);
    }
} // namespace pulsared