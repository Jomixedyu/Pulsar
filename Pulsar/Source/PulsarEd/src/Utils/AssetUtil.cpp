#include <PulsarEd/Utils/AssetUtil.h>
#include <PulsarEd/EditorAssetManager.h>
#include <PulsarEd/Windows/AssetEditorWindow.h>
#include <PulsarEd/Windows/EditorWindowManager.h>

namespace pulsared
{
    void AssetUtil::OpenAssetEditor(AssetObject_ref asset)
    {
        auto windowType = AssetEditorManager::GetValue(asset->GetType());
        if (!windowType)
            return;

        
        for (auto& win : EditorWindowManager::GetOpeningWindows(windowType))
        {
            if (sptr_cast<AssetEditorWindow>(win)->GetAssetObject() == asset)
            {
                //todo raise
                return;
            }
        }

        auto win = sptr_cast<AssetEditorWindow>(windowType->CreateSharedInstance({}));
        win->SetAssetObject(asset);
        win->Open();
    }
}