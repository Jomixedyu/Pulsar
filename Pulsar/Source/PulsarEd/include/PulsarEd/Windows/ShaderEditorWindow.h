#pragma once
#include "AssetEditorWindow.h"
#include "AssetPreviewEditorWindow.h"


namespace pulsared
{
    class ShaderEditorWindow : public AssetPreviewEditorWindow
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::ShaderEditorWindow, AssetPreviewEditorWindow);
        DEFINE_ASSET_EDITOR(Shader);
    public:
        virtual string_view GetWindowDisplayName() const { return "Shader Editor"; }

    protected:
        virtual void OnDrawImGui(float dt) override;
        virtual void OnOpen() override;
        virtual void OnClose() override;
        void OnRefreshMenuContexts() override;
        void OnDrawAssetPropertiesUI() override;

        Material_ref m_previewMaterial;
    };
}