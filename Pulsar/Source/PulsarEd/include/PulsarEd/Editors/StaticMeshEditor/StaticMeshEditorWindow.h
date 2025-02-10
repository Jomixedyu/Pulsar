#pragma once
#include "Editors/AssetEditor/AssetPreviewEditorWindow.h"
#include <Pulsar/Assets/StaticMesh.h>

namespace pulsared
{

    class StaticMeshEditorWindow : public AssetPreviewEditorWindow
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::StaticMeshEditorWindow, AssetPreviewEditorWindow);
        DEFINE_ASSET_EDITOR(StaticMesh, false);
    public:
        virtual string_view GetWindowDisplayName() const override { return "StaticMesh Editor"; }
    protected:
        virtual void OnDrawAssetPropertiesUI(float dt) override;
        virtual void OnOpen() override;
        virtual void OnClose() override;
        void OnRefreshMenuContexts() override;

    protected:
        RCPtr<StaticMesh> m_staticmesh;
    };
}