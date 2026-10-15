#include <PulsarEd/Editors/DataAssetEditor/DataAssetEditorWindow.h>

#include <PulsarEd/AssetDatabase.h>
#include <PulsarEd/PropertyControls/PropertyControl.h>

namespace pulsared
{
    SPtr<EditorWindow> DataAssetEditor::OnCreateEditorWindow()
    {
        return mksptr(new DataAssetEditorWindow());
    }

    void DataAssetEditorWindow::OnDrawAssetEditor(float dt)
    {
        base::OnDrawAssetEditor(dt);

        if (!m_assetObject)
            return;

        auto* assetType = m_assetObject->GetType();
        if (PImGui::ObjectFieldProperties(assetType, assetType, m_assetObject.GetPtr(), m_assetObject.GetPtr()))
        {
            AssetDatabase::MarkDirty(m_assetObject);
        }
    }
} // namespace pulsared
