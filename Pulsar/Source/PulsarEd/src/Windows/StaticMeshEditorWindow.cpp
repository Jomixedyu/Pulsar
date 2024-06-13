#include "Windows/StaticMeshEditorWindow.h"
#include "Pulsar/Assets/Texture2D.h"
#include "Pulsar/BuiltinAsset.h"
#include "Pulsar/Components/CameraComponent.h"
#include "Pulsar/Components/StaticMeshRendererComponent.h"
#include "Pulsar/Scene.h"

#include <PulsarEd/Menus/Menu.h>
#include <PulsarEd/Menus/MenuEntry.h>
#include <PulsarEd/Menus/MenuEntrySubMenu.h>
#include <PulsarEd/Menus/MenuRenderer.h>
#include <PulsarEd/PropertyControls/PropertyControl.h>
#include <PulsarEd/Shaders/EditorShader.h>

namespace pulsared
{

    void StaticMeshEditorWindow::OnDrawAssetPropertiesUI(float dt)
    {
        base::OnDrawAssetPropertiesUI(dt);

        if (PImGui::PropertyGroup("Mesh Info"))
        {
            if (PImGui::BeginPropertyLines())
            {
                auto info = std::format("extent: {}, radius: {}",
                    to_string(m_staticmesh->GetBounds().Extent),
                    to_string(m_staticmesh->GetBounds().Radius));

                PImGui::PropertyLineText("Boudning Box", info);
                if (ImGui::Button("Calc"))
                {
                    m_staticmesh->CalcBounds();
                    AssetDatabase::MarkDirty(m_assetObject);
                }
                PImGui::EndPropertyLines();
            }
        }
    }
    void StaticMeshEditorWindow::OnOpen()
    {
        base::OnOpen();

        m_staticmesh = cref_cast<StaticMesh>(m_assetObject);
        m_staticmesh->CreateGPUResource();

        auto previewMesh = m_world->GetResidentScene()->NewNode("PreviewMesh");
        auto renderer = previewMesh->AddComponent<StaticMeshRendererComponent>();
        renderer->SetStaticMesh(m_staticmesh);

        for (int i = 0; i < renderer->GetMaterialCount(); ++i)
        {
            renderer->SetMaterial(i, GetAssetManager()->LoadAsset<Material>(BuiltinAsset::Material_Lambert));
        }
    }
    void StaticMeshEditorWindow::OnClose()
    {
        base::OnClose();
        if (m_staticmesh)
        {
            m_staticmesh.Reset();
        }
    }

    void StaticMeshEditorWindow::OnRefreshMenuContexts()
    {
        base::OnRefreshMenuContexts();
        // m_menuBarCtxs->Contexts.push_back();
    }
} // namespace pulsared