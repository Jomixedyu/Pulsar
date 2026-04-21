#include "Editors/StaticMeshEditor/StaticMeshEditorWindow.h"
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

namespace pulsared
{

    void StaticMeshEditorWindow::OnDrawAssetPropertiesUI(float dt)
    {
        base::OnDrawAssetPropertiesUI(dt);

        if (PImGui::PropertyGroup("Mesh Info"))
        {
            if (PImGui::BeginPropertyLines())
            {
                PImGui::PropertyLineText("Extent", to_string(m_staticmesh->GetBounds().Extent));
                PImGui::PropertyLineText("Radius", std::to_string(m_staticmesh->GetBounds().Radius));
                if (ImGui::Button("Calc"))
                {
                    m_staticmesh->CalcBounds();
                    AssetDatabase::MarkDirty(m_assetObject);
                }

                size_t totalVertices = 0;
                size_t totalIndices = 0;
                for (size_t i = 0; i < m_staticmesh->GetMeshSectionCount(); i++)
                {
                    auto& section = m_staticmesh->GetMeshSection(i);
                    totalVertices += section.Positions.size();
                    totalIndices += section.Indices.size();
                }
                PImGui::PropertyLineText("Sections", std::to_string(m_staticmesh->GetMeshSectionCount()));
                PImGui::PropertyLineText("Vertices", std::to_string(totalVertices));
                PImGui::PropertyLineText("Indices", std::to_string(totalIndices));

                PImGui::EndPropertyLines();
            }
        }

        if (PImGui::PropertyGroup("Vertex Attributes"))
        {
            if (PImGui::BeginPropertyLines())
            {
                for (size_t i = 0; i < m_staticmesh->GetMeshSectionCount(); i++)
                {
                    auto& section = m_staticmesh->GetMeshSection(i);
                    if (m_staticmesh->GetMeshSectionCount() > 1)
                    {
                        ImGui::SeparatorText(std::format("Section {}", i).c_str());
                    }

                    bool hasPosition = !section.Positions.empty();
                    bool hasNormal   = !section.Normals.empty();
                    bool hasTangent  = !section.Tangents.empty();
                    bool hasColor    = !section.Colors.empty();
                    int   uvCount   = 0;
                    for (size_t uv = 0; uv < section.TexCoords.size(); uv++)
                    {
                        if (!section.TexCoords[uv].empty())
                            uvCount = (int)uv + 1;
                    }

                    PImGui::PropertyLineText("Position", hasPosition ? "Yes" : "No");
                    PImGui::PropertyLineText("Normal",   hasNormal   ? "Yes" : "No");
                    PImGui::PropertyLineText("Tangent",  hasTangent  ? "Yes" : "No");
                    PImGui::PropertyLineText("Color",    hasColor    ? "Yes" : "No");
                    PImGui::PropertyLineText("UV Sets",  uvCount > 0 ? std::to_string(uvCount) : "No");
                }
                PImGui::EndPropertyLines();
            }
        }
    }
    void StaticMeshEditorWindow::OnOpen()
    {
        base::OnOpen();

        m_staticmesh = cast<StaticMesh>(m_assetObject);
        m_staticmesh->CreateGPUResource();

        auto previewMesh = m_world->GetResidentScene()->NewNode("PreviewMesh");
        auto renderer = previewMesh->AddComponent<StaticMeshRendererComponent>();
        renderer->SetStaticMesh(m_staticmesh);

        for (int i = 0; i < renderer->GetMaterialCount(); ++i)
        {
            renderer->SetMaterial(i, AssetManager::Get()->LoadAsset<Material>(BuiltinAsset::Material_Lambert));
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