#pragma once
#include "AssetEditorWindow.h"

namespace pulsared
{
    class AssetPreviewEditorWindow : public AssetEditorWindow
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::AssetPreviewEditorWindow, AssetEditorWindow);
    public:
        virtual void OnDrawAssetPropertiesUI();
        void OnDrawImGui() override;
    };
}