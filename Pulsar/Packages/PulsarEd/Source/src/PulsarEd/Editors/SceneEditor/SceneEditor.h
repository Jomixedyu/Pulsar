#pragma once
#include "Editors/Editor.h"
#include <Pulsar/World.h>
#include <stack>

namespace pulsared
{
    class SceneEditor : public Editor
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::SceneEditor, Editor);
        DEFINE_EDITOR(SceneEditor);
        using base = Editor;
    public:
        ~SceneEditor();
        void Initialize() override;
        void Terminate() override;
        SPtr<EditorWindow> OnCreateEditorWindow() override;
        string_view GetMenuName() const override { return "SceneEditor";}

        void NewScene();
        void OpenScene();
        void SaveScene();
        bool SaveSceneAs();

        void PushPreviewWorld(std::unique_ptr<World> world);
        bool PreviewWorldStackEmpty() const;
        void PopPreviewWorld();
        World* GetPreviewWorld() const;

        void BeginPlayInEditor();
        void EndPlayInEditor();

        void BeginEditorSimulate();
        void EndEditorSimulate();
        bool IsEditorSimulating() const;

        void RouteInput(const std::vector<uinput::InputEvent>& events) override;

        static SceneEditor* GetCurrent() { return s_current; }
        static void SetCurrent(SceneEditor* editor) { s_current = editor; }

    private:
        std::stack<std::unique_ptr<World>> m_previewWorldStack;
        static SceneEditor* s_current;
    };
} // namespace pulsared
