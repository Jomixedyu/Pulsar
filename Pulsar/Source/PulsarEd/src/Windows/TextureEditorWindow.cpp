#include "Windows/TextureEditorWindow.h"

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
        base::OnOpen();
        Texture2D_ref tex = m_assetObject;
        tex->CreateGPUResource();
        gfx::GFXDescriptorSetLayoutInfo info{
            gfx::GFXDescriptorSetLayoutInfo(gfx::GFXDescriptorType::CombinedImageSampler, gfx::GFXShaderStageFlags::Fragment, 0)};
        m_descLayout = Application::GetGfxApp()->CreateDescriptorSetLayout(&info, 1);

        m_imageDesc = Application::GetGfxApp()->GetDescriptorManager()->GetDescriptorSet(m_descLayout);
        m_imageDesc->AddDescriptor("p", 0)
            ->SetTextureSampler2D(tex->GetGFXTexture().get());
        m_imageDesc->Submit();
    }
    void TextureEditorWindow::OnClose()
    {
        base::OnClose();
        m_imageDesc.reset();
        m_descLayout.reset();
    }

    void TextureEditorWindow::OnRefreshMenuContexts()
    {
        base::OnRefreshMenuContexts();
    }
    void TextureEditorWindow::OnDrawAssetPreviewUI(float dt)
    {
        base::OnDrawAssetPreviewUI(dt);
        Texture2D_ref tex = m_assetObject;

        ImGui::Image((void*)m_imageDesc->GetId(), {30,30});
    }

    void TextureEditorWindow::OnDrawAssetPropertiesUI(float dt)
    {
        base::OnDrawAssetPropertiesUI(dt);

    }

} // namespace pulsared