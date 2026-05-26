#pragma once
#include "Editors/AssetEditor/AssetEditor.h"
#include "Editors/AssetEditor/AssetPreviewEditorWindow.h"
#include "Pulsar/Assets/RenderTexture.h"
#include "Pulsar/Assets/VolumeProfile.h"

#include <Pulsar/Assets/Texture2D.h>

namespace pulsared
{
    class TextureEditor : public AssetEditor
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::TextureEditor, AssetEditor);
        DEFINE_ASSET_EDITOR_DERIVE(Texture2D);
    public:
        SPtr<EditorWindow> OnCreateEditorWindow() override;
        string_view GetMenuName() const override { return "Texture Editor"; }
    };

    class TextureEditorWindow : public AssetPreviewEditorWindow
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::TextureEditorWindow, AssetPreviewEditorWindow);
    public:
        virtual string_view GetWindowDisplayName() const { return "Texutre Editor"; }

        ~TextureEditorWindow() override;
    protected:
        virtual void OnOpen() override;
        virtual void OnClose() override;

        void OnRefreshMenuContexts() override;
        void OnDrawAssetPropertiesUI(float dt) override;
        void OnDrawAssetPreviewUI(float dt) override;

        float m_imageScale{1};
        bool m_enableTransparency{};
        bool m_enableChannelR{true};
        bool m_enableChannelG{true};
        bool m_enableChannelB{true};
        bool m_enableChannelA{true};

        RCPtr<Material> m_ppMat;
        ObjectPtr<Node> m_previewVolumeNode;
        RCPtr<VolumeProfile> m_previewProfile;
        SPtr<DisplayEncodingSettings> m_displayEncodingSettings;
    };

    inline SPtr<EditorWindow> TextureEditor::OnCreateEditorWindow()
    {
        return mksptr(new TextureEditorWindow());
    }
}