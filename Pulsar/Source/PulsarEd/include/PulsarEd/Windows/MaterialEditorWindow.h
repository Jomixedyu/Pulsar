#pragma once
#include "AssetEditorWindow.h"
#include "AssetPreviewEditorWindow.h"

namespace pulsared
{

    class MaterialEditorWindow : public AssetPreviewEditorWindow
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::MaterialEditorWindow, AssetPreviewEditorWindow);
        DEFINE_ASSET_EDITOR(Material);
    public:
        virtual string_view GetWindowDisplayName() const { return "Material Editor"; }
    protected:
        virtual void OnOpen() override;
        virtual void OnClose() override;
        void OnRefreshMenuContexts() override;
    };
}