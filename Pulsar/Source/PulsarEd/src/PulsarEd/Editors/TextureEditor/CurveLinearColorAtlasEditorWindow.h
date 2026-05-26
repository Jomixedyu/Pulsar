#pragma once
#include "Editors/AssetEditor/AssetEditor.h"
#include "Pulsar/Assets/CurveLinearColorAtlas.h"
#include "TextureEditorWindow.h"

namespace pulsared
{
    class CurveLinearColorAtlasEditor : public AssetEditor
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::CurveLinearColorAtlasEditor, AssetEditor);
        DEFINE_ASSET_EDITOR_DERIVE(CurveLinearColorAtlas);
    public:
        SPtr<EditorWindow> OnCreateEditorWindow() override;
        string_view GetMenuName() const override { return "Curve Linear Color Atlas Editor"; }
    };

    class CurveLinearColorAtlasEditorWindow : public TextureEditorWindow
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::CurveLinearColorAtlasEditorWindow, TextureEditorWindow);
    public:

    };

    inline SPtr<EditorWindow> CurveLinearColorAtlasEditor::OnCreateEditorWindow()
    {
        return mksptr(new CurveLinearColorAtlasEditorWindow());
    }
} // namespace pulsared