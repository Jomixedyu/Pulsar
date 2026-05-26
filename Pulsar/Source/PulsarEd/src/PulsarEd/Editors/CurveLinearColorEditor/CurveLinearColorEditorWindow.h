#pragma once
#include "Editors/AssetEditor/AssetEditor.h"
#include "ImGradientHDR.h"
#include "Pulsar/Assets/CurveLinearColor.h"
#include "PulsarEd/Editors/CurveEditor/CurveEditorWindow.h"

namespace pulsared
{
    class CurveLinearColorEditor : public AssetEditor
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::CurveLinearColorEditor, AssetEditor);
        DEFINE_ASSET_EDITOR_DERIVE(CurveLinearColor);
    public:
        SPtr<EditorWindow> OnCreateEditorWindow() override;
        string_view GetMenuName() const override { return "Curve Linear Color Editor"; }
    };

    class CurveLinearColorEditorWindow : public CurveEditorWindow
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::CurveLinearColorEditorWindow, CurveEditorWindow);
    public:

        void OnOpen() override;
        void OnClose() override;

        void OnDrawImGui(float dt) override;
        ImGuiWindowClass GetGuiWindowClass() const override;

    private:
        void ImportFromUEJson();

        RCPtr<CurveLinearColor> m_colorCurve;
        ImGradientHDRTemporaryState temporaryState{};
    };

    inline SPtr<EditorWindow> CurveLinearColorEditor::OnCreateEditorWindow()
    {
        return mksptr(new CurveLinearColorEditorWindow());
    }
} // namespace pulsared