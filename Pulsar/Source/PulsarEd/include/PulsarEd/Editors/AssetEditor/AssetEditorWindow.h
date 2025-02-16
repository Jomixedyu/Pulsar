#pragma once
#include "Editors/EditorWindow.h"
#include <Pulsar/AssetObject.h>
#include <PulsarEd/Utils/AssetEditorManager.h>
#include "AssetDatabase.h"
#include <Pulsar/AssetManager.h>

namespace pulsared
{
    class AssetEditorMenuContext : public MenuContextBase
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::AssetEditorMenuContext, MenuContextBase);

    public:
        explicit AssetEditorMenuContext(RCPtr<AssetObject> asset)
            : Asset(asset)
        {
        }
        RCPtr<AssetObject> Asset;
    };

    class AssetEditorWindow : public EditorWindow
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::AssetEditorWindow, EditorWindow);
    public:

        virtual ImGuiWindowFlags GetGuiWindowFlags() const override;
        string_view GetWindowDisplayName() const override;
        AssetEditorWindow();
        virtual bool IsDirtyAsset() const;
        virtual string GetWindowName() const override;
    public:
        virtual void OnDrawAssetEditor(float dt);

        RCPtr<AssetObject> GetAssetObject() const { return m_assetObject; }
        void SetAssetObject(RCPtr<AssetObject> asset) { m_assetObject = asset; }

        void OnDrawImGui(float dt) override;
        virtual void OnRefreshMenuContexts();
    protected:
        RCPtr<AssetObject> m_assetObject;
        MenuContexts_sp m_menuBarCtxs;
    };


}