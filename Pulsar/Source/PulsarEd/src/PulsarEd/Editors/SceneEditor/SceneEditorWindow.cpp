#include "Editors/SceneEditor/SceneEditorWindow.h"

#include "EditorAppInstance.h"
#include "EditorWorld.h"
#include "Editors/SceneEditor/SceneEditor.h"
#include "Editors/CommonPanel/OutlinerWindow.h"
#include "Editors/CommonPanel/PropertiesWindow.h"
#include "Editors/CommonPanel/SceneWindow.h"
#include "Editors/CommonPanel/WorkspaceWindow.h"
#include "Editors/CommonPanel/OutputWindow.h"

namespace pulsared
{

    void SceneEditorWindow::OnOpen()
    {
        m_windowDisplayName = "Scene Editor";
        base::OnOpen();

        OpenPanel(cltypeof<SceneWindow>());
        OpenPanel(cltypeof<OutlinerWindow>());
        OpenPanel(cltypeof<PropertiesWindow>());
        OpenPanel(cltypeof<WorkspaceWindow>());
        OpenPanel(cltypeof<OutputWindow>());
    }
    void SceneEditorWindow::OnClose()
    {
        base::OnClose();
    }
    void SceneEditorWindow::OnDrawImGui(float dt)
    {
        auto sceneEditor = dynamic_cast<SceneEditor*>(GetEditor());
        if (sceneEditor && ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows))
        {
            SceneEditor::SetCurrent(sceneEditor);
        }

        base::OnDrawImGui(dt);
        if (ImGui::IsKeyDown(ImGuiKey::ImGuiKey_LeftCtrl) && ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_S, false))
        {
            if (sceneEditor)
            {
                sceneEditor->SaveScene();
            }
        }
        if (ImGui::BeginMenuBar())
        {
            ImGui::Separator();
            if (ImGui::Button(ICON_FK_FLOPPY_O)) // save button
            {
                if (sceneEditor)
                {
                    sceneEditor->SaveScene();
                }
            }

            if (sceneEditor)
            {
                bool isPlaying = !sceneEditor->PreviewWorldStackEmpty();

                ImGui::BeginDisabled(isPlaying);
                if (ImGui::Button(ICON_FK_PLAY))
                {
                    sceneEditor->BeginPlayInEditor();
                }
                ImGui::EndDisabled();

                ImGui::BeginDisabled(!isPlaying);
                if (ImGui::Button(ICON_FK_STOP))
                {
                    sceneEditor->EndPlayInEditor();
                }
                ImGui::EndDisabled();
            }

            ImGui::EndMenuBar();
        }
    }

} // namespace pulsared