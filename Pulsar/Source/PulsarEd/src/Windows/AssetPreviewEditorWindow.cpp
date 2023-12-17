#include "Windows/AssetPreviewEditorWindow.h"

#include "Components/StdEditCameraControllerComponent.h"
#include "EditorWorld.h"
#include "PropertyControls/PropertyControl.h"
#include "Pulsar/Components/DirectionalLightComponent.h"
#include "Pulsar/Components/StaticMeshRendererComponent.h"
#include "Pulsar/EngineAppInstance.h"
#include "Pulsar/Scene.h"
#include "UIControls/ViewportFrame.h"

namespace pulsared
{

    void AssetPreviewEditorWindow::OnDrawAssetPropertiesUI(float dt)
    {
        if (PImGui::PropertyGroup("Asset Info"))
        {
            if (PImGui::BeginPropertyItem("Asset Path"))
            {
                ImGui::Text(AssetDatabase::GetPathByAsset(m_assetObject).c_str());
                PImGui::EndPropertyItem();
            }
            if (PImGui::BeginPropertyItem("Object Id"))
            {
                ImGui::Text(m_assetObject.handle.to_string().c_str());
                PImGui::EndPropertyItem();
            }
        }
    }
    void AssetPreviewEditorWindow::OnDrawAssetPreviewUI(float dt)
    {
        base::OnDrawAssetPreviewUI(dt);
        m_world->Tick(dt);
        m_viewportFrame.Render(dt);
    }

    void AssetPreviewEditorWindow::OnOpen()
    {
        base::OnOpen();

        auto worldName = StringUtil::Concat(GetWindowDisplayName(), " - ", std::to_string(GetWindowId()));
        m_world = new EditorWorld(worldName);
        m_world->OnWorldBegin();

        m_world->GetPersistentScene()
            ->NewNode("Light")
            ->AddComponent<DirectionalLightComponent>()
            ->GetAttachedNode()->GetTransform()
            ->TranslateRotateEuler({3,3,-3}, {45,45,0});

        //m_world->GetPreviewCamera()->GetAttachedNode()->GetTransform()->GetParent()->RotateEuler({20,0,0});
        auto gfxpipe = Application::GetGfxApp()->GetRenderPipeline();
        auto pipe = dynamic_cast<EngineRenderPipeline*>(gfxpipe);
        pipe->AddWorld(m_world);

        m_viewportFrame.Initialize();
        m_viewportFrame.SetWorld(m_world);
    }
    void AssetPreviewEditorWindow::OnClose()
    {
        AssetEditorWindow::OnClose();

        m_viewportFrame.Terminate();

        auto gfxpipe = Application::GetGfxApp()->GetRenderPipeline();
        auto pipe = dynamic_cast<EngineRenderPipeline*>(gfxpipe);

        pipe->RemoveWorld(m_world);
        m_world->OnWorldEnd();
        delete m_world;
        m_world = nullptr;
    }
} // namespace pulsared