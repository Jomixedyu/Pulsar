#include "EdTools/MeshVertexBrush.h"
#include "EdTools/MoveEdTool.h"
#include "EdTools/RotationEdTool.h"
#include "EdTools/ScaleEdTool.h"
#include "EdTools/SelectorEdTool.h"
#include "EditorAppInstance.h"
#include "EditorWorld.h"
#include "ImGuiExt.h"
#include "Subsystems/EditorGridSubsystem.h"

#include <Pulsar/Assets/Shader.h>
#include <Pulsar/Components/CameraComponent.h>
#include <Pulsar/Rendering/RenderContext.h>
#include <Pulsar/Scene.h>
#include <PulsarEd/Assembly.h>
#include <PulsarEd/Components/Grid3DComponent.h>
#include <PulsarEd/Components/StdEditCameraControllerComponent.h>
#include <PulsarEd/EditorNode.h>
#include <PulsarEd/Importers/FBXImporter.h>
#include <PulsarEd/Windows/SceneWindow.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_vulkan.h>

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


    void SceneWindow::OnDrawImGui(float dt)
    {
        static bool b = true;
        // ImGui::ShowDemoWindow(&b);

        auto world = dynamic_cast<EditorWorld*>(EditorWorld::GetPreviewWorld());

        m_sceneEditor->SetWorld(world);

        if (ImGui::BeginMenuBar())
        {
            if (!EditorWorld::PreviewWorldStackEmpty())
            {
                if (ImGui::Button("Pop"))
                {
                    EditorWorld::PopPreviewWorld();
                    world = dynamic_cast<EditorWorld*>(EditorWorld::GetPreviewWorld());
                    m_sceneEditor->SetWorld(world);
                }
            }

            const char* items[] = { "Shade" };

            ImGui::Text("Draw Mode");

            ImGui::SetNextItemWidth(150);
            if (ImGui::BeginCombo("##Draw Mode", items[this->drawmode_select_index]))
            {
                for (size_t i = 0; i < sizeof(items) / sizeof(const char*); i++)
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

            const char* editMode[] = {"SceneEditor"};
            ImGui::Text("Edit Mode");
            ImGui::SetNextItemWidth(150);
            if (ImGui::BeginCombo("##Edit Mode", editMode[this->m_editModeIndex]))
            {
                for (size_t i = 0; i < sizeof(editMode) / sizeof(const char*); i++)
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

            if (ImGui::Button(ICON_FK_ARROWS " Gizmos###Gizmos"))
            {
                world->GetGizmosManager().SetEnabled(!world->GetGizmosManager().GetEnabled());
            }

            if (ImGui::Button(ICON_FK_CUBE " 2D###2D"))
            {
                auto cam = world->GetCurrentCamera();
                auto ctrl = cam->GetNode()->GetParent()->GetComponent<StdEditCameraControllerComponent>().GetPtr();
                ctrl->SetEnable2DMode(!ctrl->GetEnable2DMode());

                auto* storeData = &ctrl->m_saved3d;

                if (ctrl->GetEnable2DMode())
                {
                    storeData = &ctrl->m_saved3d;
                }
                else
                {
                    storeData = &ctrl->m_saved2d;
                }
                auto ctrlTransform = ctrl->GetTransform();
                auto camTransform = cam->GetTransform();
                storeData->ControllerPos = ctrlTransform->GetPosition();
                storeData->ControllerEuler = ctrlTransform->GetEuler();
                storeData->CamPos = camTransform->GetPosition();
                storeData->CamEuler = camTransform->GetEuler();
                storeData->ProjectionMode = cam->GetProjectionMode();

                auto* loadData = &ctrl->m_saved3d;
                if (ctrl->GetEnable2DMode())
                {
                    loadData = &ctrl->m_saved2d;
                }
                else
                {
                    loadData = &ctrl->m_saved3d;
                }
                ctrlTransform->SetPosition(loadData->ControllerPos);
                ctrlTransform->SetEuler(loadData->ControllerEuler);
                camTransform->SetPosition(loadData->CamPos);
                camTransform->SetEuler(loadData->CamEuler);
                cam->SetProjectionMode(loadData->ProjectionMode);

            }
            if (ImGui::Button(ICON_FK_TABLE " Grid###Grid"))
            {
                // static index_string name = "__ReferenceGrid3d";
                auto grid = world->GetSubsystem<EditorGridSubsystem>();
                grid->SetVisible(!grid->IsVisible());
            }

            ImGui::EndMenuBar();
        }

        if (m_sceneEditor)
        {
            m_sceneEditor->Render(dt);
        }

        if (ImGui::IsWindowFocused())
        {

            if (ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_Q, false))
            {
                world->SetTool(std::make_unique<SelectorEdTool>());
            }
            else if (ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_W, false))
            {
                world->SetTool(std::make_unique<MoveEdTool>());
            }
            else if (ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_E, false))
            {
                world->SetTool(std::make_unique<RotationEdTool>());
            }
            else if (ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_R, false))
            {
                world->SetTool(std::make_unique<ScaleEdTool>());
            }
            else if (ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_B, false))
            {
                world->SetTool(std::make_unique<MeshVertexBrush>());
            }
            if (ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_Delete, false))
            {
                auto selection = world->GetSelection().GetSelection();

                for (auto& item : selection)
                {
                    if (item)
                    {
                        item->GetRuntimeOwnerScene()->RemoveNode(item);
                    }
                }
            }

        }

    }

    void SceneWindow::OnWindowResize()
    {
        if (!Workspace::IsOpened())
        {
            return;
        }
    }
} // namespace pulsared