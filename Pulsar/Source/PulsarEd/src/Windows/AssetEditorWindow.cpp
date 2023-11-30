#include <PulsarEd/Windows/AssetEditorWindow.h>
#include <PulsarEd/AssetDatabase.h>

namespace pulsared
{
    AssetEditorWindow::AssetEditorWindow()
    {
        m_winSize = {800, 550};
    }
    bool AssetEditorWindow::IsDirtyAsset() const
    {
        if (m_assetObject)
        {
            return AssetDatabase::IsDirty(m_assetObject);
        }
        return false;
    }

    string AssetEditorWindow::GetWindowName() const
    {
        return std::to_string(GetWindowId());
    }
}