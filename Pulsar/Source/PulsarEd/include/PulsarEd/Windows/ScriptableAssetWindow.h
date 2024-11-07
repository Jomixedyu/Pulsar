#pragma once
#include "AssetEditorWindow.h"
#include <Pulsar/Assets/ScriptableAsset.h>

namespace pulsared
{
    class ScriptableAssetWindow : public AssetEditorWindow
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::ScriptableAssetWindow, AssetEditorWindow);
        DEFINE_ASSET_EDITOR(ScriptableAsset, true);


    protected:
        void OnOpen() override;
        void OnDrawAssetEditor(float dt) override;

    };
}