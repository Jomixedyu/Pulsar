#pragma once
#include "AssetEditorWindow.h"

namespace pulsared
{

    class ShaderEditorWindow : public AssetEditorWindow
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::ShaderEditorWindow, AssetEditorWindow);
        DEFINE_ASSET_EDITOR(Shader);

    public:
        virtual string_view GetWindowDisplayName() const { return "Shader Editor"; }

    protected:
        virtual void OnDrawImGui() override;
        virtual void OnOpen() override;
        virtual void OnClose() override {}
    };

    class MaterialEditorWindow : public AssetEditorWindow
    {

    };
}