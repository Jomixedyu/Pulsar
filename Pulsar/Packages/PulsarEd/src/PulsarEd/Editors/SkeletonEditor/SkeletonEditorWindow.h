#pragma once
#include "Editors/AssetEditor/AssetEditor.h"
#include "Editors/AssetEditor/AssetEditorWindow.h"
#include <Pulsar/Assets/Skeleton.h>

namespace pulsared
{
    class SkeletonEditor : public AssetEditor
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::SkeletonEditor, AssetEditor);
        DEFINE_ASSET_EDITOR(Skeleton);
    public:
        SPtr<EditorWindow> OnCreateEditorWindow() override;
        string_view GetMenuName() const override { return "Skeleton Editor"; }
    };

    class SkeletonEditorWindow : public AssetEditorWindow
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::SkeletonEditorWindow, AssetEditorWindow);
    public:
        string_view GetWindowDisplayName() const override { return "Skeleton Editor"; }
    protected:
        void OnOpen() override;
        void OnDrawAssetEditor(float dt) override;
    private:
        RCPtr<pulsar::Skeleton> m_skeleton;
    };

    inline SPtr<EditorWindow> SkeletonEditor::OnCreateEditorWindow()
    {
        return mksptr(new SkeletonEditorWindow());
    }
}
