#include "Pulsar/Prefab.h"
#include "Pulsar/Scene.h"
#include "Pulsar/World.h"

#include "Utils/PrefabUtil.h"
#include <PulsarEd/EditorAppInstance.h>
#include <PulsarEd/EditorAssetManager.h>
#include <PulsarEd/Editors/AssetEditor/AssetEditorWindow.h>
#include <PulsarEd/Editors/EditorRegistry.h>
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
            auto world = GetEdApp()->GetEditorWorld();
            if (auto oldScene = world->GetFocusScene())
            {
                world->UnloadScene(oldScene);
            }
            scene->SetObjectFlags(scene->GetObjectFlags() & ~pulsar::OF_Transient);
            world->LoadScene(scene);
            world->SetFocusScene(scene);
            return;
        }

        auto editorType = GetValue(asset->GetType());
        if (!editorType)
        {
            Logger::Log("asset editor not found: " + asset->GetType()->GetShortName());
            return;
        }

        auto* editor = EditorRegistry::GetEditor(editorType);
        if (!editor)
        {
            Logger::Log("asset editor instance not found: " + editorType->GetShortName());
            return;
        }
        auto win = editor->CreateEditorWindow();
        auto assetWin = sptr_cast<AssetEditorWindow>(win);
        if (!assetWin)
        {
            Logger::Log("asset editor window type invalid: " + editorType->GetShortName());
            return;
        }

        for (auto& openWin : EditorWindowManager::GetOpeningWindows(win->GetType()))
        {
            auto openAssetWin = sptr_cast<AssetEditorWindow>(openWin);
            if (openAssetWin && openAssetWin->GetAssetObject() == asset)
            {
                return;
            }
        }

        assetWin->SetAssetObject(asset);
        assetWin->Open();
    }


}