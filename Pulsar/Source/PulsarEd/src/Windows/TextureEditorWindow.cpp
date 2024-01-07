#include "Windows/TextureEditorWindow.h"

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

    void TextureEditorWindow::OnOpen()
    {
        m_createDirectionalLight = false;
        base::OnOpen();

        Texture2D_ref tex = m_assetObject;
        tex->CreateGPUResource();

        m_ppMat = GetAssetManager()->LoadAsset<Material>("Engine/Materials/ImagePreview");
        m_ppMat->SetTexture("_Image"_idxstr, tex);
        m_ppMat->CreateGPUResource();
        // texPreviewPP->SubmitParameters();

        auto camera = m_viewportFrame.GetWorld()->GetPreviewCamera();
        camera->m_postProcessMaterials->push_back(m_ppMat);

        m_enableGamma = tex->IsSRGB();
    }
    void TextureEditorWindow::OnClose()
    {
        base::OnClose();
    }

    void TextureEditorWindow::OnRefreshMenuContexts()
    {
        base::OnRefreshMenuContexts();
    }

    enum RGBAButtonBits
    {
        RGBAButtonBits_None = 0,
        RGBAButtonBits_R = 1,
        RGBAButtonBits_G = 2,
        RGBAButtonBits_B = 4,
        RGBAButtonBits_A = 8,
    };
    static int RGBAButtons(bool* r, bool* g, bool* b, bool* a)
    {
        int changed = 0;
        ImVec4 normalBtnColor{0.3, 0.3, 0.3, 1};
        ImVec4 rBtnColor = normalBtnColor;
        if (r == nullptr)
        {
            ImGui::BeginDisabled();
        }
        else if (*r)
        {
            rBtnColor = ImVec4{0.7, 0, 0, 1};
        }
        ImGui::PushStyleColor(ImGuiCol_Button, rBtnColor);
        if (ImGui::Button("R"))
        {
            *r = !*r;
            changed |= RGBAButtonBits_R;
        }
        ImGui::PopStyleColor();
        if (r == nullptr)
            ImGui::EndDisabled();
        ImGui::SameLine();

        ImVec4 gBtnColor = normalBtnColor;
        if (g == nullptr)
        {
            ImGui::BeginDisabled();
        }
        else if (*g)
        {
            gBtnColor = ImVec4{0, 0.7, 0, 1};
        }
        ImGui::PushStyleColor(ImGuiCol_Button, gBtnColor);
        if (ImGui::Button("G"))
        {
            *g = !*g;
            changed |= RGBAButtonBits_G;
        }
        ImGui::PopStyleColor();
        if (g == nullptr)
            ImGui::EndDisabled();
        ImGui::SameLine();

        ImVec4 bBtnColor = normalBtnColor;
        if (b == nullptr)
        {
            ImGui::BeginDisabled();
        }
        else if (*b)
        {
            bBtnColor = ImVec4{0, 0, 0.8, 1};
        }
        ImGui::PushStyleColor(ImGuiCol_Button, bBtnColor);
        if (ImGui::Button("B"))
        {
            *b = !*b;
            changed |= RGBAButtonBits_B;
        }
        ImGui::PopStyleColor();
        if (b == nullptr)
            ImGui::EndDisabled();
        ImGui::SameLine();

        ImVec4 aBtnColor = normalBtnColor;
        if (a == nullptr)
        {
            ImGui::BeginDisabled();
        }
        else if (*a)
        {
            aBtnColor = ImVec4{0.5, 0.5, 0.5, 1};
        }
        ImGui::PushStyleColor(ImGuiCol_Button, aBtnColor);
        if (ImGui::Button("A"))
        {
            *a = !*a;
            changed |= RGBAButtonBits_A;
        }
        ImGui::PopStyleColor();
        if (a == nullptr)
            ImGui::EndDisabled();

        return changed;
    }

    #define FLAGS_SRGB 1
    #define FLAGS_EnableCheckerBackground 2
    #define FLAGS_CHANNEL_R 4
    #define FLAGS_CHANNEL_G 8
    #define FLAGS_CHANNEL_B 16
    #define FLAGS_CHANNEL_A 32
    void TextureEditorWindow::OnDrawAssetPreviewUI(float dt)
    {
        Texture2D_ref tex = m_assetObject;
        auto width = tex->GetWidth();
        auto height = tex->GetHeight();


        bool fitBtnDown = ImGui::Button("Fit");

        ImGui::SameLine();
        if (ImGui::Button("-"))
        {
            if (m_imageScale > 0.2f)
                m_imageScale -= 0.1f;
        }
        ImGui::SameLine();

        ImGui::Text(" %.2f%s ", m_imageScale * 100, "%");
        ImGui::SameLine();
        if (ImGui::Button("+"))
        {
            m_imageScale += 0.1f;
        }
        ImGui::SameLine();
        if(auto result = RGBAButtons(&m_enableChannelR, &m_enableChannelG, &m_enableChannelB, &m_enableChannelA))
        {
            int flag = m_ppMat->GetIntScalar("_Flags"_idxstr);
            if (result & RGBAButtonBits_R)
            {
                if (m_enableChannelR)
                    flag |= FLAGS_CHANNEL_R;
                else
                    flag &= ~FLAGS_CHANNEL_R;
            }
            if (result & RGBAButtonBits_G)
            {
                if (m_enableChannelG)
                    flag |= FLAGS_CHANNEL_G;
                else
                    flag &= ~FLAGS_CHANNEL_G;
            }
            if (result & RGBAButtonBits_B)
            {
                if (m_enableChannelB)
                    flag |= FLAGS_CHANNEL_B;
                else
                    flag &= ~FLAGS_CHANNEL_B;
            }
            if (result & RGBAButtonBits_A)
            {
                if (m_enableChannelA)
                    flag |= FLAGS_CHANNEL_A;
                else
                    flag &= ~FLAGS_CHANNEL_A;
            }
            m_ppMat->SetIntScalar("_Flags"_idxstr, flag);
            m_ppMat->SubmitParameters();
        }
        ImGui::SameLine();
        if (ImGui::Checkbox("Gamma2.2", &m_enableGamma))
        {
            int flag = m_ppMat->GetIntScalar("_Flags"_idxstr);
            if (m_enableGamma)
                flag |= FLAGS_SRGB;
            else
                flag &= ~FLAGS_SRGB;
            m_ppMat->SetIntScalar("_Flags"_idxstr, flag);
            m_ppMat->SubmitParameters();
        }
        ImGui::SameLine();
        if (ImGui::Checkbox("Background", &m_enableTransparency))
        {
            int flag = m_ppMat->GetIntScalar("_Flags"_idxstr);
            if (m_enableTransparency)
                flag |= FLAGS_EnableCheckerBackground;
            else
                flag &= ~FLAGS_EnableCheckerBackground;
            m_ppMat->SetIntScalar("_Flags"_idxstr, flag);
            m_ppMat->SubmitParameters();
        }

        auto size = ImGui::GetContentRegionAvail();
        if (fitBtnDown)
        {
            float rateX = size.x / width;
            float rateY = size.y / height;
            m_imageScale = std::min(rateX, rateY);
        }


        //ImGui::SetNextWindowPos(ImGui::GetWindowPos() + size / 2 - (ImVec2(width, height) / 2) * m_imageScale);
        if (ImGui::BeginChild("pic", {width * m_imageScale, height * m_imageScale}))
        {

            base::OnDrawAssetPreviewUI(dt);
        }
        ImGui::EndChild();
    }

    void TextureEditorWindow::OnDrawAssetPropertiesUI(float dt)
    {
        base::OnDrawAssetPropertiesUI(dt);

        if (PImGui::PropertyGroup("Asset Info"))
        {
            if (PImGui::BeginPropertyLine())
            {
                PImGui::PropertyLineText("Object Handle", m_assetObject.GetHandle().to_string());
                PImGui::EndPropertyLine();
            }
        }
    }

} // namespace pulsared