#include "Windows/SpriteEditorWindow.h"

#include "PropertyControls/PropertyControl.h"

namespace pulsared
{
    SpriteEditorWindow::~SpriteEditorWindow()
    {
    }
    void SpriteEditorWindow::OnOpen()
    {
        base::OnOpen();
    }
    void SpriteEditorWindow::OnClose()
    {
        base::OnClose();
    }
    void SpriteEditorWindow::OnDrawAssetPropertiesUI(float dt)
    {
        base::OnDrawAssetPropertiesUI(dt);
        RCPtr<SpriteAtlas> atlas = cref_cast<SpriteAtlas>(m_assetObject);

        if (PImGui::PropertyGroup("Sprite"))
        {
            PImGui::ObjectFieldProperties(
                BoxingObjectPtrBase::StaticType(),
                m_assetObject->GetType(),
                mkbox(ObjectPtrBase(m_assetObject.GetHandle())).get(),
                m_assetObject.GetPtr());
        }
    }
    void SpriteEditorWindow::OnDrawAssetPreviewUI(float dt)
    {
        base::OnDrawAssetPreviewUI(dt);
    }
} // namespace pulsared