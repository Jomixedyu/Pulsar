#pragma once
#include "Editors/AssetEditor/AssetEditor.h"
#include "Editors/AssetEditor/AssetPreviewEditorWindow.h"

namespace pulsared
{
    class MaterialEditor : public AssetEditor
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::MaterialEditor, AssetEditor);
        DEFINE_ASSET_EDITOR(Material);
    public:
        SPtr<EditorWindow> OnCreateEditorWindow() override;
        string_view GetMenuName() const override { return "Material Editor"; }
    };

    class MaterialEditorWindow : public AssetPreviewEditorWindow
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::MaterialEditorWindow, AssetPreviewEditorWindow);
    public:
        virtual string_view GetWindowDisplayName() const { return "Material Editor"; }
    protected:
        virtual void OnDrawAssetPropertiesUI(float dt) override;
        virtual void OnOpen() override;
        virtual void OnClose() override;
        void OnRefreshMenuContexts() override;

        void OnShaderChanged(const RCPtr<Shader>& newShader);
    protected:
        ObjectPtr<StaticMeshRendererComponent> m_previewMeshRenderer;
        RCPtr<Shader> m_shader;
    };

    inline SPtr<EditorWindow> MaterialEditor::OnCreateEditorWindow()
    {
        return mksptr(new MaterialEditorWindow());
    }
}
