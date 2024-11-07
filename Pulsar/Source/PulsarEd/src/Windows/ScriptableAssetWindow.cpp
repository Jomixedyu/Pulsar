#include "Windows/ScriptableAssetWindow.h"

#include "PropertyControls/PropertyControl.h"

namespace pulsared
{

    void ScriptableAssetWindow::OnOpen()
    {
        base::OnOpen();
        m_windowDisplayName = "AssetEditor - ";
        auto assetPath = AssetDatabase::GetPathByAsset(m_assetObject);
        m_windowDisplayName.append(assetPath);
    }
    void ScriptableAssetWindow::OnDrawAssetEditor(float dt)
    {
        Type* assetType = m_assetObject->GetType();

        base::OnDrawAssetEditor(dt);
        PImGui::ObjectFieldProperties(
            assetType,
            assetType,
            m_assetObject.GetPtr(),
            m_assetObject.GetPtr());
    }
} // namespace pulsared