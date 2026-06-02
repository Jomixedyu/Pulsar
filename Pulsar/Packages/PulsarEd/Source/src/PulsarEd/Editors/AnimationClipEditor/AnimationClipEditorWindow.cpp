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
                PImGui::EndPropertyLines();
            }
        }

        // --- 轨道列表 ---
        if (PImGui::PropertyGroup("Tracks"))
        {
            if (ImGui::BeginTable("TracksTable", 5,
                ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY,
                ImVec2(0, 200)))
            {
                ImGui::TableSetupScrollFreeze(0, 1);
                ImGui::TableSetupColumn("Type",       ImGuiTableColumnFlags_WidthFixed, 80.f);
                ImGui::TableSetupColumn("Name",       ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Binding",    ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Keys",       ImGuiTableColumnFlags_WidthFixed, 120.f);
                ImGui::TableSetupColumn("",           ImGuiTableColumnFlags_WidthFixed, 40.f);
                ImGui::TableHeadersRow();

                for (auto& trackBase : m_clip->GetTracks())
                {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    if (trackBase->TrackType == pulsar::AnimationTrackType::Bone)
                    {
                        ImGui::TextUnformatted("Bone");
                        ImGui::TableSetColumnIndex(1);
                        auto boneTrack = pulsar::sptr_cast<pulsar::BoneAnimationTrack>(trackBase);
                        ImGui::TextUnformatted(boneTrack ? boneTrack->BoneName.c_str() : "");
                        ImGui::TableSetColumnIndex(2);
                        ImGui::Text("-");
                        ImGui::TableSetColumnIndex(3);
                        if (boneTrack)
                        {
                            ImGui::Text("P:%d R:%d S:%d",
                                (int)boneTrack->PositionKeys.size(),
                                (int)boneTrack->RotationKeys.size(),
                                (int)boneTrack->ScaleKeys.size());
                        }
                    }
                    else
                    {
                        ImGui::TextUnformatted("Property");
                        ImGui::TableSetColumnIndex(1);
                        auto propTrack = pulsar::sptr_cast<pulsar::PropertyAnimationTrack>(trackBase);
                        ImGui::TextUnformatted(propTrack ? propTrack->FieldName.c_str() : "");
                        ImGui::TableSetColumnIndex(2);
                        if (propTrack)
                            ImGui::Text("%s::%s", propTrack->ComponentTypeName.c_str(), propTrack->FieldName.c_str());
                        ImGui::TableSetColumnIndex(3);
                        if (propTrack)
                        {
                            ImGui::Text("F:%d V3:%d Q:%d",
                                (int)propTrack->FloatKeys.size(),
                                (int)propTrack->Vector3Keys.size(),
                                (int)propTrack->QuatKeys.size());
                        }
                    }
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
