#pragma once
#include "Editors/AssetEditor/AssetEditor.h"
#include "Editors/AssetEditor/AssetPreviewEditorWindow.h"
#include <Pulsar/Assets/StaticMesh.h>

namespace pulsared
{

    class StaticMeshEditor : public AssetEditor
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::StaticMeshEditor, AssetEditor);
        DEFINE_ASSET_EDITOR(StaticMesh);
    public:
        SPtr<EditorWindow> OnCreateEditorWindow() override;
        string_view GetMenuName() const override { return "StaticMesh Editor"; }
    };

    class StaticMeshEditorWindow : public AssetPreviewEditorWindow
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::StaticMeshEditorWindow, AssetPreviewEditorWindow);
    public:
        virtual string_view GetWindowDisplayName() const override { return "StaticMesh Editor"; }
    protected:
        virtual void OnDrawAssetPropertiesUI(float dt) override;
        virtual void OnOpen() override;
        virtual void OnClose() override;
        void OnRefreshMenuContexts() override;

    protected:
        RCPtr<StaticMesh> m_staticmesh;
        int m_uvPreviewChannel = 0;
    };

    inline SPtr<EditorWindow> StaticMeshEditor::OnCreateEditorWindow()
    {
        return mksptr(new StaticMeshEditorWindow());
    }
}