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

        auto texPreviewPP = GetAssetManager()->LoadAsset<Material>("Engine/Materials/ImagePreview");
        texPreviewPP->SetTexture("_Image"_idxstr, tex);
        texPreviewPP->SubmitParameters();

        auto camera = m_viewportFrame.GetWorld()->GetPreviewCamera();
        camera->m_postProcessMaterials->push_back(texPreviewPP);


    }
    void TextureEditorWindow::OnClose()
    {
        base::OnClose();
    }

    void TextureEditorWindow::OnRefreshMenuContexts()
    {
        base::OnRefreshMenuContexts();
    }
    void TextureEditorWindow::OnDrawAssetPreviewUI(float dt)
    {

        Texture2D_ref tex = m_assetObject;

        //ImGui::Image((void*)m_imageDesc->GetId(), {300,300});

        if (ImGui::Button("Fit"))
        {

        }
        ImGui::SameLine();
        if (ImGui::Button("-"))
        {

        }
        ImGui::SameLine();

        ImGui::Text(" %s ", "100%");
        ImGui::SameLine();
        if (ImGui::Button("+"))
        {

        }
        
        base::OnDrawAssetPreviewUI(dt);
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