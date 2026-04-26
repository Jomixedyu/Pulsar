#pragma once
#include "Editors/AssetEditor/AssetEditorWindow.h"
#include <Pulsar/Assets/Skeleton.h>

namespace pulsared
{
    class SkeletonEditorWindow : public AssetEditorWindow
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::SkeletonEditorWindow, AssetEditorWindow);
        DEFINE_ASSET_EDITOR(Skeleton, false);
    public:
        string_view GetWindowDisplayName() const override { return "Skeleton Editor"; }
    protected:
        void OnOpen() override;
        void OnDrawAssetEditor(float dt) override;
    private:
        RCPtr<pulsar::Skeleton> m_skeleton;
    };
}
