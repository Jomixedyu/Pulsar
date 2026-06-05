#pragma once
#include "Editors/AssetEditor/AssetEditor.h"
#include "Editors/AssetEditor/AssetEditorWindow.h"
#include <Pulsar/Assets/ScriptableAsset.h>

namespace pulsared
{
    class ScriptableAssetEditor : public AssetEditor
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::ScriptableAssetEditor, AssetEditor);
        DEFINE_ASSET_EDITOR_DERIVE(ScriptableAsset);
    public:
        SPtr<EditorWindow> OnCreateEditorWindow() override;
        string_view GetMenuName() const override { return "Scriptable Asset Editor"; }
    };

    class ScriptableAssetWindow : public AssetEditorWindow
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::ScriptableAssetWindow, AssetEditorWindow);

    protected:
        void OnOpen() override;
        void OnDrawAssetEditor(float dt) override;

    };

    inline SPtr<EditorWindow> ScriptableAssetEditor::OnCreateEditorWindow()
    {
        return mksptr(new ScriptableAssetWindow());
    }
}