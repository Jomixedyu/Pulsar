#pragma once
#include "EditorWindow.h"
#include <Apatite/Components/CameraComponent.h>

namespace apatiteed
{
    class SceneWindow : public EditorWindow
    {
        CORELIB_DEF_TYPE(AssemblyObject_ApatiteEd, apatiteed::SceneWindow, EditorWindow);
    public:
        virtual ImGuiWindowFlags GetGuiWindowFlags() const override {
            return ImGuiWindowFlags_None | ImGuiWindowFlags_::ImGuiWindowFlags_MenuBar;
        }
        SceneWindow();
        virtual void OnOpen() override;
        virtual void OnClose() override;
        static string_view StaticWindowName() { return "Scene"; }
        virtual string_view GetWindowName() const override { return StaticWindowName(); }
        virtual void OnDrawImGui() override;


        Node_sp GetSceneCameraNode() { return this->camera_node; }
        CameraComponent_sp GetSceneCamera() { return this->camera_node->GetComponent<CameraComponent>(); }
    private:
        virtual void OnWindowResize();
    private:
        int32_t drawmode_select_index = 0;
        Node_sp camera_node;
        Vector2i win_size_;
    };
}