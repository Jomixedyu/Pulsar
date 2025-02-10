#include "Editors/AssetEditor/AssetPreviewEditorWindow.h"

#include "Components/StdEditCameraControllerComponent.h"
#include "EdTools/ViewEdTool.h"
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
        if (PImGui::PropertyGroup("Asset Object"))
        {
            if (PImGui::BeginPropertyLines())
            {
                PImGui::PropertyLineText("Asset Path", AssetDatabase::GetPathByAsset(m_assetObject));
                PImGui::PropertyLineText("Asset Id", m_assetObject.GetHandle().to_string());

                PImGui::EndPropertyLines();
            }
        }
    }
    void AssetPreviewEditorWindow::OnDrawAssetPreviewUI(float dt)
    {
        m_world->Tick(dt);
        m_viewportFrame.Render(dt);
    }
    void AssetPreviewEditorWindow::OnDrawAssetEditor(float dt)
    {
        base::OnDrawAssetEditor(dt);

        if (!m_assetObject)
        {
            ImGui::Text("no asset");
            return;
        }
        ImGui::Columns(2);
        if (ImGui::BeginChild("#Preview"))
        {
            OnDrawAssetPreviewUI(dt);
        }
        ImGui::EndChild();

        ImGui::NextColumn();
        if (ImGui::BeginChild("Properties"))
        {
            OnDrawAssetPropertiesUI(dt);
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

        m_world->GetResidentScene()
            ->NewNode("Light")
            ->AddComponent<DirectionalLightComponent>()
            ->GetNode()->GetTransform()
            ->TranslateRotateEuler({3,3,-3}, {45,45,0});

        //m_world->GetPreviewCamera()->GetAttachedNode()->GetTransform()->GetParent()->RotateEuler({20,0,0});
        auto gfxpipe = Application::GetGfxApp()->GetRenderPipeline();
        auto pipe = dynamic_cast<EngineRenderPipeline*>(gfxpipe);
        pipe->AddWorld(m_world);

        m_viewportFrame.Initialize();
        m_viewportFrame.SetWorld(m_world);
        dynamic_cast<EditorWorld*>(m_world)->SetTool(std::make_unique<ViewEdTool>());
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