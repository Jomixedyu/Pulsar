#pragma once
#include "AssetEditorWindow.h"
#include "AssetPreviewEditorWindow.h"

namespace pulsared
{

    class MaterialEditorWindow : public AssetPreviewEditorWindow
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::MaterialEditorWindow, AssetPreviewEditorWindow);
        DEFINE_ASSET_EDITOR(Material, false);
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
}