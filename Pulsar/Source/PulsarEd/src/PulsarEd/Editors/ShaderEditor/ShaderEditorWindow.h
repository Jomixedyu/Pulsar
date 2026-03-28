#pragma once
#include "Editors/AssetEditor/AssetPreviewEditorWindow.h"


namespace pulsared
{
    class ShaderEditorWindow : public AssetEditorWindow
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::ShaderEditorWindow, AssetEditorWindow);
        DEFINE_ASSET_EDITOR(Shader, false);
    public:
        virtual string_view GetWindowDisplayName() const override { return "Shader Editor"; }

    protected:
        virtual void OnDrawImGui(float dt) override;
        virtual void OnOpen() override;
        virtual void OnClose() override;
        void OnRefreshMenuContexts() override;

    };
}