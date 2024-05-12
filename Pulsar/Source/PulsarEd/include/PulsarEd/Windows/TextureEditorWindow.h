#pragma once
#include "AssetEditorWindow.h"
#include "AssetPreviewEditorWindow.h"
#include "Pulsar/Assets/RenderTexture.h"

#include <Pulsar/Assets/Texture2D.h>

namespace pulsared
{
    class TextureEditorWindow : public AssetPreviewEditorWindow
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::TextureEditorWindow, AssetPreviewEditorWindow);
        DEFINE_ASSET_EDITOR(Texture2D);
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
        bool m_enableGamma{};
        bool m_enableTransparency{};
        bool m_enableChannelR{true};
        bool m_enableChannelG{true};
        bool m_enableChannelB{true};
        bool m_enableChannelA{true};

        RCPtr<Material> m_ppMat;
    };
}