#pragma once
#include "EdTools/EdTool.h"
#include "Windows/PanelWindow.h"

#include <Pulsar/Components/CameraComponent.h>
#include <Pulsar/Node.h>
#include <PulsarEd/UIControls/ViewportFrame.h>

namespace pulsared
{


    class SceneWindow : public PanelWindow
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::SceneWindow, PanelWindow);
    public:
        virtual ImGuiWindowFlags GetGuiWindowFlags() const override {
            return ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_::ImGuiWindowFlags_MenuBar;
        }
        SceneWindow();
        virtual void OnOpen() override;
        virtual void OnClose() override;
        static string_view StaticWindowName() { return ICON_FK_MAP " Scene" "###Scene"; }
        virtual string_view GetWindowDisplayName() const override { return StaticWindowName(); }
        virtual void OnDrawImGui(float dt) override;


        //ObjectPtr<Node> GetSceneCameraNode() { return this->m_camNode; }
        //ObjectPtr<Node> GetSceneCameraControllerNode() { return this->m_camCtrlNode; }
        //CameraComponent_ref GetSceneCamera() { return this->GetSceneCameraNode()->GetComponent<CameraComponent>(); }

    private:
        virtual void OnWindowResize();

        void OnOpenWorkspace();
        void OnCloseWorkspace();

        void CaptureScreenshot();
    private:
        int32_t drawmode_select_index = 0;
        std::unique_ptr<SceneEditorViewportFrame> m_sceneEditor;


        //ObjectPtr<Node> m_camNode;
        //ObjectPtr<Node> m_camCtrlNode;
        //Vector2i m_viewportSize;

        //gfx::GFXDescriptorSetLayout_sp m_descriptorLayout;
        //gfx::GFXDescriptorSet_sp m_descriptorSet;
    };
}
