#include "Pulsar/Prefab.h"
#include "Pulsar/Scene.h"
#include "Pulsar/World.h"

#include "Utils/PrefabUtil.h"
#include <PulsarEd/EditorAssetManager.h>
#include <PulsarEd/Editors/AssetEditor/AssetEditorWindow.h>
#include <PulsarEd/Utils/AssetEditorManager.h>
#include <PulsarEd/Windows/EditorWindowManager.h>

namespace pulsared
{

    void AssetEditorManager::OpenAssetEditor(RCPtr<AssetObject> asset)
    {
        if (auto prefab = cast<Prefab>(asset))
        {
            PrefabUtil::OpenPrefab(prefab);
            return;
        }

        if (auto scene = cast<pulsar::Scene>(asset))
        {
            auto world = pulsar::World::Current();
            if (auto oldScene = world->GetFocusScene())
            {
                world->UnloadScene(oldScene);
            }
            scene->SetObjectFlags(scene->GetObjectFlags() & ~pulsar::OF_Transient);
            world->LoadScene(scene);
            world->SetFocusScene(scene);
            return;
        }

        auto windowType = GetValue(asset->GetType());
        if (!windowType)
        {
            Logger::Log("asset editor not found: " + asset->GetType()->GetShortName());
            return;
        }

        
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