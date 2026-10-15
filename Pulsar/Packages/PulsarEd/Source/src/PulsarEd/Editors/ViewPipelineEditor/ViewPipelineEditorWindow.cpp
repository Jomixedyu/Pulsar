#include "Editors/ViewPipelineEditor/ViewPipelineEditorWindow.h"
#include "AssetDatabase.h"
#include "PropertyControls/PropertyControl.h"
#include "UIControls/TypePicker.h"
#include <Pulsar/IconsForkAwesome.h>
#include <imgui/imgui.h>
#include <exception>

namespace pulsared
{
    static void MarkPipelineModified(const RCPtr<ViewPipelineSettings>& pipeline)
    {
        AssetDatabase::MarkDirty(pipeline);
        pipeline->NotifyModified();
    }

    void ViewPipelineEditorWindow::OnOpen()
    {
        base::OnOpen();
        m_windowDisplayName = "ViewPipeline - ";
        m_windowDisplayName.append(AssetDatabase::GetPathByAsset(m_assetObject));
    }

    void ViewPipelineEditorWindow::OnDrawAssetEditor(float dt)
    {
        base::OnDrawAssetEditor(dt);

        auto pipeline = cast<ViewPipelineSettings>(m_assetObject);
        if (!pipeline)
            return;

        DrawFeatureList(pipeline);
    }

    void ViewPipelineEditorWindow::DrawFeatureList(const RCPtr<ViewPipelineSettings>& pipeline)
    {
        auto& features = *pipeline->GetFeatures();

        ImGui::Separator();
        ImGui::Text("Render Features");
        ImGui::SameLine();

        if (ImGui::Button(ICON_FK_PLUS " Add Feature"))
            TypePickerPopup::Open("AddRenderFeaturePopup");

        Type* selectedType = nullptr;
        TypePickerOptions options{};
        options.BaseType = cltypeof<RenderFeatureSettings>();
        options.IncludeBaseType = false;
        options.Filter = [](Type* type)
        {
            try
            {
                auto object = type->CreateSharedInstance({});
                return sptr_cast<RenderFeatureSettings>(object) != nullptr;
            }
            catch (const std::exception&)
            {
                return false;
            }
        };

        if (TypePickerPopup::Draw("AddRenderFeaturePopup", options, selectedType))
        {
            try
            {
                auto object = selectedType->CreateSharedInstance({});
                if (auto feature = sptr_cast<RenderFeatureSettings>(object))
                {
                    features.push_back(feature);
                    MarkPipelineModified(pipeline);
                }
            }
            catch (const std::exception&)
            {
            }
        }

        for (int index = 0; index < static_cast<int>(features.size()); ++index)
        {
            auto& feature = features[index];
            ImGui::PushID(index);

            const char* featureName = feature ? feature->GetType()->GetShortName().c_str() : "[Null]";
            bool opened = ImGui::TreeNodeEx(featureName, ImGuiTreeNodeFlags_DefaultOpen);

            ImGui::SameLine();
            if (ImGui::SmallButton(ICON_FK_TRASH))
            {
                features.erase(features.begin() + index);
                MarkPipelineModified(pipeline);
                if (opened)
                    ImGui::TreePop();
                ImGui::PopID();
                --index;
                continue;
            }

            ImGui::SameLine();
            if (ImGui::SmallButton(ICON_FK_ARROW_UP) && index > 0)
            {
                std::swap(features[index], features[index - 1]);
                MarkPipelineModified(pipeline);
            }

            ImGui::SameLine();
            if (ImGui::SmallButton(ICON_FK_ARROW_DOWN) && index + 1 < static_cast<int>(features.size()))
            {
                std::swap(features[index], features[index + 1]);
                MarkPipelineModified(pipeline);
            }

            if (opened)
            {
                if (feature && PImGui::ObjectFieldProperties(feature->GetType(), feature->GetType(), feature.get(), feature.get()))
                    MarkPipelineModified(pipeline);
                ImGui::TreePop();
            }

            ImGui::PopID();
        }
    }
}
