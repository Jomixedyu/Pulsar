#pragma once

#include <Pulsar/Assets/DataAsset.h>
#include <PulsarEd/Editors/AssetEditor/AssetEditor.h>
#include <PulsarEd/Editors/AssetEditor/AssetEditorWindow.h>

namespace pulsared
{
    class DataAssetEditor : public AssetEditor
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::DataAssetEditor, AssetEditor);
        DEFINE_ASSET_EDITOR_DERIVE(DataAsset);

    public:
        SPtr<EditorWindow> OnCreateEditorWindow() override;
        string_view GetMenuName() const override { return "Data Asset Editor"; }
    };

    class DataAssetEditorWindow : public AssetEditorWindow
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::DataAssetEditorWindow, AssetEditorWindow);

    protected:
        void OnDrawAssetEditor(float dt) override;
    };
} // namespace pulsared
