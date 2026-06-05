#pragma once
#include "Editors/AssetEditor/AssetEditor.h"
#include "Editors/AssetEditor/AssetPreviewEditorWindow.h"
#include "Pulsar/Assets/RenderTexture.h"
#include <Pulsar/Assets/SpriteAtlas.h>

#include <Pulsar/Assets/Texture2D.h>

namespace pulsared
{
    class SpriteEditor : public AssetEditor
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::SpriteEditor, AssetEditor);
        DEFINE_ASSET_EDITOR_DERIVE(SpriteAtlas);
    public:
        SPtr<EditorWindow> OnCreateEditorWindow() override;
        string_view GetMenuName() const override { return "Sprite Editor"; }
    };

    class SpriteEditorWindow : public AssetPreviewEditorWindow
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::SpriteEditorWindow, AssetPreviewEditorWindow);
    public:
        virtual string_view GetWindowDisplayName() const { return "Sprite Editor"; }

        ~SpriteEditorWindow() override;
    protected:
        virtual void OnOpen() override;
        virtual void OnClose() override;

        void OnDrawAssetPropertiesUI(float dt) override;
        void OnDrawAssetPreviewUI(float dt) override;

        //RCPtr<Material> m_ppMat;
    };

    inline SPtr<EditorWindow> SpriteEditor::OnCreateEditorWindow()
    {
        return mksptr(new SpriteEditorWindow());
    }
}