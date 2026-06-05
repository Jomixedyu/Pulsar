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

        if (PImGui::PropertyGroup("Debug Visualization"))
        {
            auto previewNode = m_world->GetResidentScene()->FindNodeByName("PreviewMesh");
            if (previewNode)
            {
                auto renderer = previewNode->GetComponent<StaticMeshRendererComponent>();
                if (renderer)
                {
                    if (PImGui::BeginPropertyLines())
                    {
                        bool showN = renderer->GetShowNormalGizmo();
                        bool showT = renderer->GetShowTangentGizmo();
                        bool showB = renderer->GetShowBitangentGizmo();

                        auto _DrawBoolLine = [](const char* label, bool& value)
                        {
                            ImGui::TableNextRow();
                            ImGui::TableSetColumnIndex(0);
                            ImGui::AlignTextToFramePadding();
                            bool treeOpened = ImGui::TreeNodeEx(label, ImGuiTreeNodeFlags_Leaf);
                            ImGui::TableSetColumnIndex(1);
                            ImGui::AlignTextToFramePadding();
                            bool changed = ImGui::Checkbox((std::string("##") + label).c_str(), &value);
                            if (treeOpened) ImGui::TreePop();
                            return changed;
                        };

                        if (_DrawBoolLine("Show Normals", showN))
                            renderer->SetShowNormalGizmo(showN);
                        if (_DrawBoolLine("Show Tangents", showT))
                            renderer->SetShowTangentGizmo(showT);
                        if (_DrawBoolLine("Show Bitangents", showB))
                            renderer->SetShowBitangentGizmo(showB);

                        PImGui::EndPropertyLines();
                    }
                }
            }
        }

        if (PImGui::PropertyGroup("UV Preview"))
        {
            int maxUVChannels = 0;
            for (int si = 0; si < (int)m_staticmesh->GetMeshSectionCount(); ++si)
            {
                auto& section = m_staticmesh->GetMeshSection(si);
                for (int uv = 0; uv < (int)section.TexCoords.size(); ++uv)
                {
                    if (!section.TexCoords[uv].empty())
                        maxUVChannels = std::max(maxUVChannels, uv + 1);
                }
            }

            if (maxUVChannels > 0)
            {
                if (m_uvPreviewChannel >= maxUVChannels)
                    m_uvPreviewChannel = maxUVChannels - 1;

                if (maxUVChannels > 1)
                {
                    ImGui::Text("Channel:");
                    ImGui::SameLine();
                    for (int c = 0; c < maxUVChannels; ++c)
                    {
                        if (c > 0) ImGui::SameLine();
                        if (ImGui::RadioButton(std::format("UV{}", c).c_str(), &m_uvPreviewChannel, c))
                        {
                        }
                    }
                }

                const ImVec2 canvasSize(256, 256);
                const float padding = 4.0f;
                const float scale = canvasSize.x - padding * 2.0f;

                ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(20, 20, 20, 255));
                if (ImGui::BeginChild("UVCanvas", canvasSize, true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
                {
                    ImDrawList* drawList = ImGui::GetWindowDrawList();
                    ImVec2 winPos = ImGui::GetWindowPos();
                    ImVec2 contentMin = ImGui::GetWindowContentRegionMin();
                    ImVec2 origin = ImVec2(winPos.x + contentMin.x + padding, winPos.y + contentMin.y + padding);

                    // 背景网格
                    const ImU32 gridColor = IM_COL32(60, 60, 60, 255);
                    for (int i = 0; i <= 4; ++i)
                    {
                        float t = i / 4.0f;
                        drawList->AddLine(
                            ImVec2(origin.x + t * scale, origin.y),
                            ImVec2(origin.x + t * scale, origin.y + scale),
                            gridColor, 1.0f);
                        drawList->AddLine(
                            ImVec2(origin.x, origin.y + t * scale),
                            ImVec2(origin.x + scale, origin.y + t * scale),
                            gridColor, 1.0f);
                    }

                    // 外框
                    drawList->AddRect(origin, ImVec2(origin.x + scale, origin.y + scale), IM_COL32(120, 120, 120, 255), 0.0f, 0, 1.5f);

                    auto UVToScreen = [&](const Vector2f& uv) -> ImVec2
                    {
                        return ImVec2(origin.x + uv.x * scale, origin.y + (1.0f - uv.y) * scale);
                    };

                    const ImU32 lineColor = IM_COL32(0, 220, 100, 200);

                    for (int si = 0; si < (int)m_staticmesh->GetMeshSectionCount(); ++si)
                    {
                        auto& section = m_staticmesh->GetMeshSection(si);
                        if (m_uvPreviewChannel >= (int)section.TexCoords.size()) continue;
                        if (section.TexCoords[m_uvPreviewChannel].empty()) continue;
                        if (section.Indices.empty()) continue;

                        const size_t triCount = section.Indices.size() / 3;
                        for (size_t ti = 0; ti < triCount; ++ti)
                        {
                            int i0 = section.Indices[ti * 3 + 0];
                            int i1 = section.Indices[ti * 3 + 1];
                            int i2 = section.Indices[ti * 3 + 2];

                            if (i0 < 0 || i0 >= (int)section.TexCoords[m_uvPreviewChannel].size()) continue;
                            if (i1 < 0 || i1 >= (int)section.TexCoords[m_uvPreviewChannel].size()) continue;
                            if (i2 < 0 || i2 >= (int)section.TexCoords[m_uvPreviewChannel].size()) continue;

                            ImVec2 p0 = UVToScreen(section.TexCoords[m_uvPreviewChannel][i0]);
                            ImVec2 p1 = UVToScreen(section.TexCoords[m_uvPreviewChannel][i1]);
                            ImVec2 p2 = UVToScreen(section.TexCoords[m_uvPreviewChannel][i2]);

                            drawList->AddLine(p0, p1, lineColor, 1.0f);
                            drawList->AddLine(p1, p2, lineColor, 1.0f);
                            drawList->AddLine(p2, p0, lineColor, 1.0f);
                        }
                    }
                }
                ImGui::EndChild();
                ImGui::PopStyleColor();
            }
            else
            {
                ImGui::TextDisabled("No UV data available.");
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