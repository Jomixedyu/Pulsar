#include "Windows/AssetPreviewEditorWindow.h"
#include "PropertyControls/PropertyControl.h"
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
    void AssetPreviewEditorWindow::OnDrawImGui()
    {
        base::OnDrawImGui();
        if (!m_assetObject)
        {
            ImGui::Text("no asset");
            return;
        }
        ImGui::Columns(2);
        if (ImGui::BeginChild("#Preview"))
        {
            const auto descSet = AssetDatabase::IconPool->GetDescriptorSet({cltypeof<AssetObject>()->GetName()});
            const auto size = PImGui::GetContentSize();
            ImGui::Image((void*)descSet.lock()->GetId(), ImVec2(size.x, size.y));
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
}