#pragma once
#include "Pulsar/Components/StaticMeshRendererComponent.h"

#include <PulsarEd/Editors/AssetEditor/AssetEditorWindow.h>
#include <Pulsar/InputSystem/InputActionMap.h>

namespace pulsared
{
    class InputActionMapEditorWindow : public AssetEditorWindow
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::InputActionMapEditorWindow, AssetEditorWindow);
        DEFINE_ASSET_EDITOR(InputActionMap, false);
    public:
        string_view GetWindowDisplayName() const override { return "InputActionMap Editor"; }
    protected:
        void OnDrawAssetEditor(float dt) override;
        virtual void OnOpen() override;
        virtual void OnClose() override;

    protected:
        int m_selectedActionIndex = -1;
        int m_selectedBinding = -1;
    };

}