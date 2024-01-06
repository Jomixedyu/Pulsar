#pragma once
#include "AssetEditorWindow.h"
#include "UIControls/ViewportFrame.h"

namespace pulsared
{

    class AssetPreviewEditorWindow : public AssetEditorWindow
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::AssetPreviewEditorWindow, AssetEditorWindow);
    public:
        void OnDrawAssetPropertiesUI(float dt) override;
        void OnDrawAssetPreviewUI(float dt) override;
        void OnOpen() override;
        void OnClose() override;
    protected:
        World* m_world{nullptr};
        ViewportFrame m_viewportFrame;
        bool m_createDirectionalLight = true;
    };


}