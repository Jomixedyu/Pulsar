#include "EditorWorld.h"
#include "Pulsar/Prefab.h"

#include <PulsarEd/EditorAssetManager.h>
#include <PulsarEd/Utils/AssetUtil.h>
#include <PulsarEd/Windows/AssetEditorWindow.h>
#include <PulsarEd/Windows/EditorWindowManager.h>
#include "Utils/PrefabUtil.h"

namespace pulsared
{

    void AssetUtil::OpenAssetEditor(AssetObject_ref asset)
    {
        if (asset->GetType()->IsSubclassOf(cltypeof<Prefab>()))
        {
            PrefabUtil::OpenPrefab(asset);
            return;
        }

        auto windowType = AssetEditorManager::GetValue(asset->GetType());
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