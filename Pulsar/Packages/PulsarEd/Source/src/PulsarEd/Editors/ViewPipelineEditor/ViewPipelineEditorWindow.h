#pragma once
#include "Editors/AssetEditor/AssetEditor.h"
#include "Editors/AssetEditor/AssetEditorWindow.h"
#include <Pulsar/Assets/ViewPipelineSettings.h>

namespace pulsared
{
    class ViewPipelineEditor : public AssetEditor
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::ViewPipelineEditor, AssetEditor);
        DEFINE_ASSET_EDITOR(ViewPipelineSettings);
    public:
        SPtr<EditorWindow> OnCreateEditorWindow() override;
        string_view GetMenuName() const override { return "View Pipeline Editor"; }
    };

    class ViewPipelineEditorWindow : public AssetEditorWindow
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::ViewPipelineEditorWindow, AssetEditorWindow);

    protected:
        void OnOpen() override;
        void OnDrawAssetEditor(float dt) override;

    private:
        void DrawFeatureList(const RCPtr<ViewPipelineSettings>& pipeline);
    };

    inline SPtr<EditorWindow> ViewPipelineEditor::OnCreateEditorWindow()
    {
        return mksptr(new ViewPipelineEditorWindow());
    }
}