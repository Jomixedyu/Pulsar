#include <PulsarEd/Windows/SceneWindow.h>
#include <Pulsar/Assets/Shader.h>
#include <Pulsar/Scene.h>
#include <PulsarEd/Assembly.h>
#include <PulsarEd/EditorNode.h>
#include <Pulsar/Components/CameraComponent.h>
#include <PulsarEd/Importers/FBXImporter.h>
#include <PulsarEd/Components/StdEditCameraControllerComponent.h>
#include <PulsarEd/Components/Grid3DComponent.h>
#include <Pulsar/Rendering/RenderContext.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_vulkan.h>
#include <gfx-vk/GFXVulkanRenderTarget.h>
#include "EditorAppInstance.h"
#include <PulsarEd/ExclusiveTask.h>
#include <PulsarEd/Workspace.h>

namespace pulsared
{

    SceneWindow::SceneWindow()
    {

    }


    void SceneWindow::OnOpenWorkspace()
    {
        m_sceneEditor = new SceneEditorViewportFrame;
        m_sceneEditor->Initialize();
        m_sceneEditor->SetWorld(GetEdApp()->GetEditorWorld());
    }
    void SceneWindow::OnCloseWorkspace()
    {
        m_sceneEditor->Terminate();
        delete m_sceneEditor;
        m_sceneEditor = nullptr;
    }

    void SceneWindow::OnOpen()
    {
        base::OnOpen();

        Workspace::OnWorkspaceOpened.AddListener(this, &ThisClass::OnOpenWorkspace);
        Workspace::OnWorkspaceClosed.AddListener(this, &ThisClass::OnCloseWorkspace);

        if (Workspace::IsOpened())
        {
            this->OnOpenWorkspace();
        }


    }

    void SceneWindow::OnClose()
    {
        base::OnClose();

        Workspace::OnWorkspaceOpened.RemoveListener(this, &ThisClass::OnOpenWorkspace);
        Workspace::OnWorkspaceClosed.RemoveListener(this, &ThisClass::OnCloseWorkspace);

        if (Workspace::IsOpened())
        {
            this->OnCloseWorkspace();
        }


    }

    void SceneWindow::OnDrawImGui()
    {
        // ImGui::ShowDemoWindow();

        if (ImGui::BeginMenuBar())
        {
            const char* items[] = { "Shade", "Wire", "Unlit" };

            ImGui::Text("Draw Mode");

            ImGui::SetNextItemWidth(150);
            if (ImGui::BeginCombo("##Draw Mode", items[this->drawmode_select_index]))
            {
                for (size_t i = 0; i < 3; i++)
                {
                    bool selected = this->drawmode_select_index == i;
                    if (ImGui::Selectable(items[i], selected))
                    {
                        this->drawmode_select_index = static_cast<int>(i);
                        this->drawmode_select_index = 0;
                    }
                }
                ImGui::EndCombo();
            }

            const char* editMode[] = { "SceneEditor", "Modeling" };
            ImGui::Text("Edit Mode");
            ImGui::SetNextItemWidth(150);
            if (ImGui::BeginCombo("##Edit Mode", editMode[this->m_editModeIndex]))
            {
                for (size_t i = 0; i < 2; i++)
                {
                    bool selected = this->m_editModeIndex == i;
                    if (ImGui::Selectable(editMode[i], selected))
                    {
                        this->m_editModeIndex = static_cast<int>(i);
                        this->m_editModeIndex = 0;
                    }
                }
                ImGui::EndCombo();
            }

            ImGui::Button(ICON_FK_ARROWS " Gizmos###Gizmos");
            ImGui::Button(ICON_FK_CUBE " 2D###2D");
            ImGui::Button(ICON_FK_TABLE " Grid###Grid");

            ImGui::EndMenuBar();
        }

        if (m_sceneEditor)
        {
            m_sceneEditor->Render(0);
        }
    }

    void SceneWindow::OnWindowResize()
    {
        if (!Workspace::IsOpened())
        {
            return;
        }

    }

}