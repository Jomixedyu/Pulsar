#include "Editors/VolumeProfileEditor/VolumeProfileEditorWindow.h"

#include "AssetDatabase.h"
#include "DragInfo.h"
#include "PropertyControls/PropertyControl.h"
#include <Pulsar/IconsForkAwesome.h>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

namespace pulsared
{

    void VolumeProfileEditorWindow::OnOpen()
    {
        base::OnOpen();
        m_windowDisplayName = "VolumeProfile - ";
        auto assetPath = AssetDatabase::GetPathByAsset(m_assetObject);
        m_windowDisplayName.append(assetPath);
    }

    void VolumeProfileEditorWindow::OnDrawAssetEditor(float dt)
    {
        base::OnDrawAssetEditor(dt);

        auto profile = cast<VolumeProfile>(m_assetObject);
        if (!profile)
            return;

        DrawEffectsList(profile.GetPtr());
    }

    void VolumeProfileEditorWindow::DrawEffectsList(VolumeProfile* profile)
    {
        auto& effects = *profile->GetEffects();

        ImGui::Separator();
        ImGui::Text("Effects");
        ImGui::SameLine();

        if (ImGui::Button(ICON_FK_PLUS " Add Effect"))
        {
            ImGui::OpenPopup("AddEffectPopup");
        }

        if (ImGui::BeginPopup("AddEffectPopup"))
        {
            if (ImGui::MenuItem("Tonemapping"))
            {
                auto tm = mksptr(new TonemappingSettings());
                effects.push_back(tm);
                AssetDatabase::MarkDirty(m_assetObject);
            }
            if (ImGui::MenuItem("Gamma Correction"))
            {
                auto gamma = mksptr(new GammaCorrectionSettings());
                effects.push_back(gamma);
                AssetDatabase::MarkDirty(m_assetObject);
            }
            if (ImGui::MenuItem("Post Process Material"))
            {
                auto ppMat = mksptr(new PostProcessMaterialSettings());
                effects.push_back(ppMat);
                AssetDatabase::MarkDirty(m_assetObject);
            }

            ImGui::EndPopup();
        }

        for (int i = 0; i < static_cast<int>(effects.size()); ++i)
        {
            auto& effect = effects[i];
            if (!effect)
                continue;

            ImGui::PushID(i);

            auto* effectType = effect->GetType();
            bool opened = ImGui::TreeNodeEx(effectType->GetName().c_str(), ImGuiTreeNodeFlags_DefaultOpen);

            ImGui::SameLine();
            if (ImGui::SmallButton(ICON_FK_TRASH))
            {
                effects.erase(effects.begin() + i);
                AssetDatabase::MarkDirty(m_assetObject);
                ImGui::PopID();
                if (opened)
                    ImGui::TreePop();
                --i;
                continue;
            }

            if (opened)
            {
                PImGui::ObjectFieldProperties(
                    effectType,
                    effectType,
                    effect.get(),
                    effect.get());
                ImGui::TreePop();
            }

            ImGui::PopID();
        }
    }

} // namespace pulsared
