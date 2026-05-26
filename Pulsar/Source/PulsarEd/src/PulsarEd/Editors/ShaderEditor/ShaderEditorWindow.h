#pragma once
#include "Editors/AssetEditor/AssetEditor.h"
#include "Editors/AssetEditor/AssetPreviewEditorWindow.h"


namespace pulsared
{
    class ShaderEditor : public AssetEditor
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::ShaderEditor, AssetEditor);
        DEFINE_ASSET_EDITOR(Shader);
    public:
        SPtr<EditorWindow> OnCreateEditorWindow() override;
        string_view GetMenuName() const override { return "Shader Editor"; }
    };

    class ShaderEditorWindow : public AssetEditorWindow
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::ShaderEditorWindow, AssetEditorWindow);
    public:
        virtual string_view GetWindowDisplayName() const override { return "Shader Editor"; }

    protected:
        virtual void OnDrawImGui(float dt) override;
        virtual void OnOpen() override;
        virtual void OnClose() override;
        void OnRefreshMenuContexts() override;

    };

    inline SPtr<EditorWindow> ShaderEditor::OnCreateEditorWindow()
    {
        return mksptr(new ShaderEditorWindow());
    }
}