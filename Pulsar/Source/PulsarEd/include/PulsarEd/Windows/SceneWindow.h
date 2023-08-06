#pragma once
#include "EditorWindow.h"
#include <Pulsar/Components/CameraComponent.h>

namespace pulsared
{
    class SceneWindow : public EditorWindow
    {
        CORELIB_DEF_TYPE(AssemblyObject_PulsarEd, pulsared::SceneWindow, EditorWindow);
    public:
        virtual ImGuiWindowFlags GetGuiWindowFlags() const override {
            return ImGuiWindowFlags_None | ImGuiWindowFlags_::ImGuiWindowFlags_MenuBar;
        }
        SceneWindow();
        virtual void OnOpen() override;
        virtual void OnClose() override;
        static string_view StaticWindowName() { return ICON_FK_MAP " Scene" "###Scene"; }
        virtual string_view GetWindowName() const override { return StaticWindowName(); }
        virtual void OnDrawImGui() override;


        Node_ref GetSceneCameraNode() { return this->m_camNode; }
        Node_ref GetSceneCameraControllerNode() { return this->m_camCtrlNode; }
        CameraComponent_ref GetSceneCamera() { return this->GetSceneCameraNode()->GetComponent<CameraComponent>(); }
    private:
        virtual void OnWindowResize();
    private:
        int32_t drawmode_select_index = 0;
        int32_t m_editModeIndex = 0;
        Node_ref m_camNode;
        Node_ref m_camCtrlNode;
        Vector2i m_viewportSize;

        gfx::GFXDescriptorSetLayout_sp m_descriptorLayout;
        gfx::GFXDescriptorSet_sp m_descriptorSet;
    };
}