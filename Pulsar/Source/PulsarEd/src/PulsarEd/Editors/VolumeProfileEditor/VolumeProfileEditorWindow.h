#pragma once
#include "Editors/AssetEditor/AssetEditorWindow.h"
#include <Pulsar/Assets/VolumeProfile.h>

namespace pulsared
{
    class VolumeProfileEditorWindow : public AssetEditorWindow
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::VolumeProfileEditorWindow, AssetEditorWindow);
        DEFINE_ASSET_EDITOR(VolumeProfile, false);

    protected:
        void OnOpen() override;
        void OnDrawAssetEditor(float dt) override;

    public:
        static void DrawEffectsList(const pulsar::RCPtr<pulsar::VolumeProfile>& profile);
    };
}
