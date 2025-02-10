#include "Pulsar/Prefab.h"

#include "Utils/PrefabUtil.h"
#include <PulsarEd/EditorAssetManager.h>
#include <PulsarEd/Editors/AssetEditor/AssetEditorWindow.h>
#include <PulsarEd/Utils/AssetEditorManager.h>
#include <PulsarEd/Windows/EditorWindowManager.h>

namespace pulsared
{

    void AssetEditorManager::OpenAssetEditor(RCPtr<AssetObject> asset)
    {
        if (auto prefab = cref_cast<Prefab>(asset))
        {
            PrefabUtil::OpenPrefab(prefab);
            return;
        }

        auto windowType = GetValue(asset->GetType());
        if (!windowType)
            return;
        
        for (auto& win : EditorWindowManager::GetOpeningWindows(windowType))
        {
            if (sptr_cast<AssetEditorWindow>(win)->GetAssetObject() == asset)
            {
                //todo throw exceptions
                return;
            }
        }

        auto win = sptr_cast<AssetEditorWindow>(windowType->CreateSharedInstance({}));
        win->SetAssetObject(asset);
        win->Open();
    }
}