#pragma once
#include "PanelWindow.h"
#include <Pulsar/AssetObject.h>
#include <PulsarEd/EditorAssetManager.h>

namespace pulsared
{

    class AssetEditorWindow : public PanelWindow
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::AssetEditorWindow, PanelWindow);
    public:

        virtual ImGuiWindowFlags GetGuiWindowFlags() const {
            return ImGuiWindowFlags_MenuBar | (IsDirtyAsset() ? ImGuiWindowFlags_UnsavedDocument : ImGuiWindowFlags_None);
        }
        AssetEditorWindow();
        virtual bool IsDirtyAsset() const;
        virtual string GetWindowName() const override;
    public:

        AssetObject_ref GetAssetObject() const { return m_assetObject; }
        void SetAssetObject(AssetObject_ref asset) { m_assetObject = asset; }

    protected:
        AssetObject_ref m_assetObject;
    };


}