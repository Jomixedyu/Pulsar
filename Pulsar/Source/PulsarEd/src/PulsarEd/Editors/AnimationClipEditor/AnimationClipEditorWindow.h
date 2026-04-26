#pragma once
#include "Editors/AssetEditor/AssetPreviewEditorWindow.h"
#include <Pulsar/Assets/AnimationClip.h>

namespace pulsared
{
    class AnimationClipEditorWindow : public AssetPreviewEditorWindow
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::AnimationClipEditorWindow, AssetPreviewEditorWindow);
        DEFINE_ASSET_EDITOR(AnimationClip, false);
    public:
        string_view GetWindowDisplayName() const override { return "AnimationClip Editor"; }
    protected:
        void OnOpen() override;
        void OnClose() override;
        void OnDrawAssetPropertiesUI(float dt) override;
        void OnDrawAssetPreviewUI(float dt) override;
    private:
        RCPtr<pulsar::AnimationClip> m_clip;
        bool m_isPlaying = false;
        float m_previewTime = 0.f;
    };
}
