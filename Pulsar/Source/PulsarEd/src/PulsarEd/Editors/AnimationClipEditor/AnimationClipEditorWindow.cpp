#include "Editors/AnimationClipEditor/AnimationClipEditorWindow.h"
#include <Pulsar/BuiltinAsset.h>
#include <Pulsar/Assets/Material.h>
#include <Pulsar/AssetManager.h>
#include <PulsarEd/PropertyControls/PropertyControl.h>
#include <imgui/imgui.h>

namespace pulsared
{
    void AnimationClipEditorWindow::OnOpen()
    {
        base::OnOpen();
        m_clip = cast<pulsar::AnimationClip>(m_assetObject);

        // TODO: 后续支持拖入 SkinnedMesh 做带网格的预览
        // 目前只展示 Clip 数据 + 时间轴控制
    }

    void AnimationClipEditorWindow::OnClose()
    {
        m_clip.Reset();
        base::OnClose();
    }

    void AnimationClipEditorWindow::OnDrawAssetPropertiesUI(float dt)
    {
        base::OnDrawAssetPropertiesUI(dt);

        if (!m_clip) return;

        // --- 基本信息 ---
        if (PImGui::PropertyGroup("Clip Info"))
        {
            if (PImGui::BeginPropertyLines())
            {
                PImGui::PropertyLineText("Duration",
                    std::format("{:.3f} s", m_clip->GetDuration()));
                PImGui::PropertyLineText("FPS",
                    std::format("{:.1f}", m_clip->GetFPS()));
                PImGui::PropertyLineText("Tracks",
                    std::format("{}", m_clip->GetTracks().size()));
                auto skel = m_clip->GetSkeleton();
                PImGui::PropertyLineText("Skeleton",
                    skel ? skel->GetName() : "(none)");
                PImGui::EndPropertyLines();
            }
        }

        // --- 轨道列表 ---
        if (PImGui::PropertyGroup("Bone Tracks"))
        {
            if (ImGui::BeginTable("TracksTable", 4,
                ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY,
                ImVec2(0, 200)))
            {
                ImGui::TableSetupScrollFreeze(0, 1);
                ImGui::TableSetupColumn("Bone Name",  ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Pos Keys",   ImGuiTableColumnFlags_WidthFixed, 70.f);
                ImGui::TableSetupColumn("Rot Keys",   ImGuiTableColumnFlags_WidthFixed, 70.f);
                ImGui::TableSetupColumn("Scale Keys", ImGuiTableColumnFlags_WidthFixed, 75.f);
                ImGui::TableHeadersRow();

                for (auto& track : m_clip->GetTracks())
                {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted(track.BoneName.c_str());
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%d", (int)track.PositionKeys.size());
                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("%d", (int)track.RotationKeys.size());
                    ImGui::TableSetColumnIndex(3);
                    ImGui::Text("%d", (int)track.ScaleKeys.size());
                }
                ImGui::EndTable();
            }
        }
    }

    void AnimationClipEditorWindow::OnDrawAssetPreviewUI(float dt)
    {
        base::OnDrawAssetPreviewUI(dt);

        if (!m_clip) return;

        const float duration = m_clip->GetDuration();

        // --- 播放控制条 ---
        ImGui::Separator();

        // 编辑器预览没有绑定 SkinnedMeshRendererComponent，使用带 renderer 参数的重载
        // 目前预览窗口中没有 SkinnedMesh，只展示数据，播放控制仍然可以操作时间轴
        if (m_isPlaying)
        {
            m_previewTime += dt;
            if (m_previewTime > duration)
                m_previewTime = std::fmod(m_previewTime, duration > 0.f ? duration : 1.f);
        }

        if (ImGui::Button(m_isPlaying ? ICON_FK_PAUSE " Pause" : ICON_FK_PLAY " Play"))
            m_isPlaying = !m_isPlaying;
        ImGui::SameLine();
        if (ImGui::Button(ICON_FK_STOP " Stop"))
        {
            m_isPlaying = false;
            m_previewTime = 0.f;
        }
        ImGui::SameLine();
        ImGui::SetNextItemWidth(200.f);
        ImGui::SliderFloat("##time", &m_previewTime, 0.f, duration > 0.f ? duration : 1.f, "%.3f s");

        // 当前时间显示
        ImGui::SameLine();
        ImGui::Text("/ %.3f s", duration);
    }
} // namespace pulsared
