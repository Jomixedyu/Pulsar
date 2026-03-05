#pragma once
#include "Editors/Editor.h"

namespace pulsared
{
    class SceneEditor : public Editor
    {
        using base = Editor;
    public:
        void Initialize() override;
        void Terminate() override;
        SPtr<EditorWindow> OnCreateEditorWindow() override;
        string_view GetMenuName() const override { return "SceneEditor";}
    };
} // namespace pulsared