#pragma once
#include "Editors/AssetEditor/AssetPreviewEditorWindow.h"


namespace pulsared
{
    class ShaderEditorWindow : public AssetPreviewEditorWindow
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::ShaderEditorWindow, AssetPreviewEditorWindow);
        DEFINE_ASSET_EDITOR(Shader, false);
    public:
        virtual string_view GetWindowDisplayName() const { return "Shader Editor"; }

    protected:
        virtual void OnDrawImGui(float dt) override;
        virtual void OnOpen() override;
        virtual void OnClose() override;
        void OnRefreshMenuContexts() override;
        void OnDrawAssetPropertiesUI(float dt) override;

        bool m_isShaderReady{};

        RCPtr<Material> m_previewMaterial;

    };
}