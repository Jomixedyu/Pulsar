#include "Editors/TextureEditor/TextureEditorWindow.h"

#include "Pulsar/AssetManager.h"
#include "Pulsar/Assets/Material.h"
#include "Pulsar/Assets/Shader.h"
#include "Pulsar/Components/CameraComponent.h"
#include "Pulsar/Components/StaticMeshRendererComponent.h"
#include "Pulsar/Components/VolumeComponent.h"
#include "Pulsar/Assets/VolumeProfile.h"
#include "Pulsar/Node.h"
#include "Pulsar/Scene.h"

#include <PulsarEd/Menus/Menu.h>
#include <PulsarEd/Menus/MenuEntry.h>
#include <PulsarEd/Menus/MenuEntrySubMenu.h>
#include <PulsarEd/Menus/MenuRenderer.h>
#include <PulsarEd/PropertyControls/PropertyControl.h>

namespace pulsared
{
    #define FLAGS_GAMMA 0x01
    #define FLAGS_EnableCheckerBackground 0x02
    #define FLAGS_CHANNEL_R 0x04
    #define FLAGS_CHANNEL_G 0x08
    #define FLAGS_CHANNEL_B 0x10
    #define FLAGS_CHANNEL_A 0x20
    #define FLAGS_NORMALMAP 0x40

    TextureEditorWindow::~TextureEditorWindow()
    {

    }

    void TextureEditorWindow::OnOpen()
    {
        m_createDirectionalLight = false;
        base::OnOpen();

        RCPtr<Texture> tex = cast<Texture>(m_assetObject);
        tex->CreateGPUResource();

        auto shader = AssetManager::Get()->LoadAsset<Shader>("Engine/Shaders/PreviewImage");
        m_ppMat = Material::StaticCreate(shader);

        m_ppMat->SetTexture("_Image", tex);

        int32_t flags{};
        flags |= FLAGS_CHANNEL_R | FLAGS_CHANNEL_G | FLAGS_CHANNEL_B | FLAGS_CHANNEL_A;
        if (tex->GetCompressedFormat() == TextureCompressionFormat::NormalMap_Compressed)
        {
            flags |= FLAGS_NORMALMAP;
        }
        m_ppMat->SetIntScalar("_Flags", flags);
        m_ppMat->CreateGPUResource();

        if (auto* world = m_viewportFrame.GetWorld())
        {
            m_previewVolumeNode = world->GetResidentScene()->NewNode("TexturePreviewPP");
            auto volComp = m_previewVolumeNode->AddComponent<VolumeComponent>();
            volComp->SetIsGlobal(true);
            m_previewProfile = NewAssetObject<VolumeProfile>();
            auto ppMatSettings = mksptr(new PostProcessMaterialSettings());
            ppMatSettings->m_materials->push_back(m_ppMat);
            m_previewProfile->GetEffects()->push_back(ppMatSettings);
            m_gammaSettings = mksptr(new GammaCorrectionSettings());
            m_gammaSettings->m_enabled = true;
            m_previewProfile->GetEffects()->push_back(m_gammaSettings);
            volComp->SetProfile(m_previewProfile);
        }
    }

    void TextureEditorWindow::OnClose()
    {
        if (m_previewVolumeNode && m_world)
        {
            m_world->GetResidentScene()->RemoveNode(m_previewVolumeNode);
            m_previewVolumeNode.Reset();
        }
        m_ppMat.Reset();
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
        ImVec4 normalBtnColor{0.3f, 0.3f, 0.3f, 1.f};
        ImVec4 rBtnColor = normalBtnColor;
        if (r == nullptr)
        {
            ImGui::BeginDisabled();
        }
        else if (*r)
        {
            rBtnColor = ImVec4{0.7f, 0.f, 0.f, 1.f};
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
            gBtnColor = ImVec4{0.f, 0.7f, 0.f, 1.f};
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
            bBtnColor = ImVec4{0.f, 0.f, 0.8f, 1.f};
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


    void TextureEditorWindow::OnDrawAssetPreviewUI(float dt)
    {
        RCPtr<Texture> tex = cast<Texture>(m_assetObject);
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
            int flag = m_ppMat->GetIntScalar("_Flags");
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
            m_ppMat->SetIntScalar("_Flags", flag);
        }

        ImGui::SameLine();
        if (ImGui::Checkbox("bg", &m_enableTransparency))
        {
            int flag = m_ppMat->GetIntScalar("_Flags");
            if (m_enableTransparency)
                flag |= FLAGS_EnableCheckerBackground;
            else
                flag &= ~FLAGS_EnableCheckerBackground;
            m_ppMat->SetIntScalar("_Flags", flag);
        }




        auto size = ImGui::GetContentRegionAvail();
        if (fitBtnDown)
        {
            float rateX = size.x / width;
            float rateY = size.y / height;
            m_imageScale = std::min(rateX, rateY);
        }

        if (m_ppMat)
        {
            m_ppMat->SetFloat("_Zoom", m_imageScale);
            m_ppMat->SetVector4("_TexSize", Vector4f(width, height, size.x, size.y));
        }

        ImGui::BeginChild("picframe", size);

        ImVec2 frameSize = ImGui::GetContentRegionAvail();
        //ImGui::SetNextWindowPos(ImGui::GetWindowPos() + size / 2 - (ImVec2((float)width, (float)height) / 2));
        if (ImGui::BeginChild("pic", frameSize))
        {
            base::OnDrawAssetPreviewUI(dt);
            // Mouse wheel zoom (only when hovering the preview area)
            if (ImGui::IsWindowHovered())
            {
                float wheel = ImGui::GetIO().MouseWheel;
                if (wheel != 0.0f)
                {
                    constexpr float kZoomSpeed = 0.1f;
                    constexpr float kMinScale  = 0.05f;
                    constexpr float kMaxScale  = 32.0f;
                    m_imageScale *= (1.0f + wheel * kZoomSpeed);
                    m_imageScale  = std::clamp(m_imageScale, kMinScale, kMaxScale);
                }
            }
        }
        ImGui::EndChild();

        ImGui::EndChild();

        if (m_ppMat)
        {
            m_ppMat->SubmitParameters();
        }
    }

    void TextureEditorWindow::OnDrawAssetPropertiesUI(float dt)
    {
        base::OnDrawAssetPropertiesUI(dt);

        RCPtr<Texture> tex = cast<Texture>(m_assetObject);
        const auto texSize = tex->GetSize2df();

        if (PImGui::PropertyGroup("Texture"))
        {
            bool changed = PImGui::ObjectFieldProperties(
                BoxingObjectPtrBase::StaticType(),
                m_assetObject->GetType(),
                mkbox(ObjectPtrBase(m_assetObject.GetHandle())).get(),
                m_assetObject.GetPtr());


            if (changed)
            {
                AssetDatabase::MarkDirty(m_assetObject);
            }

        }

        if (PImGui::PropertyGroup("Info"))
        {
            if (PImGui::BeginPropertyLines())
            {
                PImGui::PropertyLineText("Image Size", std::format("{} x {}", texSize.x, texSize.y) );
                PImGui::PropertyLineText("Origin File Size", std::format("{} kb", tex->GetOriginCompressedBinarySize() / 1024) );
                PImGui::PropertyLineText("Raw Memory Size", std::format("{} kb", tex->GetRawBinarySize() / 1024) );
                PImGui::PropertyLineText("Compressed Size", std::format("{} kb", tex->GetNativeBinarySize() / 1024));
                PImGui::EndPropertyLines();
            }
        }

        if (PImGui::PropertyGroup("Formats (Win64)"))
        {
            if (PImGui::BeginPropertyLines())
            {
                for (auto item : *Texture2D::StaticGetFormatMapping(OSPlatform::Windows64))
                {
                    PImGui::PropertyLineText(mkbox(item.first)->GetName(), to_string(item.second));
                }
                PImGui::EndPropertyLines();
            }

        }
    }

} // namespace pulsared