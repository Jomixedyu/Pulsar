#include "Windows/AssetPreviewEditorWindow.h"

#include "Components/StdEditCameraControllerComponent.h"
#include "EditorWorld.h"
#include "PropertyControls/PropertyControl.h"
#include "Pulsar/Components/StaticMeshRendererComponent.h"
#include "Pulsar/EngineAppInstance.h"
#include "Pulsar/Scene.h"
#include "UIControls/ViewportFrame.h"

namespace pulsared
{

    void AssetPreviewEditorWindow::OnDrawAssetPropertiesUI()
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
    void AssetPreviewEditorWindow::OnDrawImGui(float dt)
    {
        base::OnDrawImGui(dt);
        if (!m_assetObject)
        {
            ImGui::Text("no asset");
            return;
        }
        ImGui::Columns(2);
        if (ImGui::BeginChild("#Preview"))
        {
            m_world->Tick(dt);
            m_viewportFrame.Render(dt);
        }
        ImGui::EndChild();

        ImGui::NextColumn();
        if (ImGui::BeginChild("Properties"))
        {
            OnDrawAssetPropertiesUI();
        }
        ImGui::EndChild();

        ImGui::Columns(1);
    }
    void AssetPreviewEditorWindow::OnOpen()
    {
        base::OnOpen();

        auto worldName = StringUtil::Concat(GetWindowDisplayName(), " - ", std::to_string(GetWindowId()));
        m_world = new EditorWorld(worldName);
        m_world->OnWorldBegin();
        {
            auto previewMesh = Node::StaticCreate("Mesh");
            auto renderer = previewMesh->AddComponent<StaticMeshRendererComponent>();

            renderer->SetStaticMesh(GetAssetManager()->LoadAsset<StaticMesh>("Engine/Shapes/Sphere"));
            renderer->SetMaterial(0, GetAssetManager()->LoadAsset<Material>("Engine/Materials/Missing"));
            m_world->GetPersistentScene()->AddNode(previewMesh);

        }
        //m_world->GetPreviewCamera()->SetBackgroundColor({0,0.5,0,1});
        //m_world->GetPreviewCamera()->GetAttachedNode()->GetTransform()->GetParent()->Translate({0,1,-3});
        // m_world->GetPreviewCamera()->GetAttachedNode()->GetTransform()->Translate({0,0,-5});
        m_world->GetPreviewCamera()->GetAttachedNode()->GetTransform()->GetParent()->RotateEuler({20,0,0});
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