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

    protected:
        virtual void OnOpen() override;
        virtual void OnClose() override;
        void OnRefreshMenuContexts() override;
        void OnDrawAssetPropertiesUI(float dt) override;
        void OnDrawAssetPreviewUI(float dt) override;

    };
}