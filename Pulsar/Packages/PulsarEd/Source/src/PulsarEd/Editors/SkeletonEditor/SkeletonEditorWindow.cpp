#include "Editors/SkeletonEditor/SkeletonEditorWindow.h"
#include <imgui/imgui.h>

namespace pulsared
{
    void SkeletonEditorWindow::OnOpen()
    {
        base::OnOpen();
        m_skeleton = cast<pulsar::Skeleton>(m_assetObject);
    }

    void SkeletonEditorWindow::OnDrawAssetEditor(float dt)
    {
        base::OnDrawAssetEditor(dt);

        if (!m_skeleton) return;

        const auto& bones = m_skeleton->GetBones();

        ImGui::Text("Bone Count: %d", (int)bones.size());
        ImGui::Separator();

        if (ImGui::BeginTable("BonesTable", 3,
            ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY))
        {
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableSetupColumn("Index",        ImGuiTableColumnFlags_WidthFixed, 50.f);
            ImGui::TableSetupColumn("Name",         ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Parent Index", ImGuiTableColumnFlags_WidthFixed, 90.f);
            ImGui::TableHeadersRow();

            for (int i = 0; i < (int)bones.size(); ++i)
            {
                const auto& bone = bones[i];
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%d", i);
                ImGui::TableSetColumnIndex(1);
                // 根据层级缩进
                int depth = 0;
                int p = bone.ParentIndex;
                while (p >= 0) { ++depth; p = bones[p].ParentIndex; }
                if (depth > 0) ImGui::Indent((float)(depth * 12));
                ImGui::TextUnformatted(bone.Name.c_str());
                if (depth > 0) ImGui::Unindent((float)(depth * 12));
                ImGui::TableSetColumnIndex(2);
                if (bone.ParentIndex < 0)
                    ImGui::TextDisabled("(root)");
                else
                    ImGui::Text("%d", bone.ParentIndex);
            }

            ImGui::EndTable();
        }
    }
} // namespace pulsared
