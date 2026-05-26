#pragma once
#include "Editors/AssetEditor/AssetEditor.h"
#include "Editors/AssetEditor/AssetEditorWindow.h"
#include <Pulsar/Assets/VolumeProfile.h>

namespace pulsared
{
    class VolumeProfileEditor : public AssetEditor
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::VolumeProfileEditor, AssetEditor);
        DEFINE_ASSET_EDITOR(VolumeProfile);
    public:
        SPtr<EditorWindow> OnCreateEditorWindow() override;
        string_view GetMenuName() const override { return "Volume Profile Editor"; }
    };

    class VolumeProfileEditorWindow : public AssetEditorWindow
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::VolumeProfileEditorWindow, AssetEditorWindow);

    protected:
        void OnOpen() override;
        void OnDrawAssetEditor(float dt) override;

    public:
        static void DrawEffectsList(const pulsar::RCPtr<pulsar::VolumeProfile>& profile);
    };

    inline SPtr<EditorWindow> VolumeProfileEditor::OnCreateEditorWindow()
    {
        return mksptr(new VolumeProfileEditorWindow());
    }

}
