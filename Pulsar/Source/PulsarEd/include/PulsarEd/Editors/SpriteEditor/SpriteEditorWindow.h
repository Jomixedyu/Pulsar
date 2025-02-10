#pragma once
#include "Editors/AssetEditor/AssetPreviewEditorWindow.h"
#include "Pulsar/Assets/RenderTexture.h"
#include <Pulsar/Assets/SpriteAtlas.h>

#include <Pulsar/Assets/Texture2D.h>

namespace pulsared
{
    class SpriteEditorWindow : public AssetPreviewEditorWindow
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::SpriteEditorWindow, AssetPreviewEditorWindow);
        DEFINE_ASSET_EDITOR(SpriteAtlas, true);
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
}