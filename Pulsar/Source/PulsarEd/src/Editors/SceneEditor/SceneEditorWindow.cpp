#include "Editors/SceneEditor/SceneEditorWindow.h"

#include "EditorAppInstance.h"
#include "Editors/CommonPanel/OutlinerWindow.h"
#include "Editors/CommonPanel/PropertiesWindow.h"
#include "Editors/CommonPanel/SceneWindow.h"
#include "Editors/CommonPanel/WorkspaceWindow.h"

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
    }
    void SceneEditorWindow::OnClose()
    {
        base::OnClose();
    }
    void SceneEditorWindow::OnDrawImGui(float dt)
    {
        base::OnDrawImGui(dt);
        if (ImGui::BeginMenuBar())
        {
            ImGui::Separator();
            ImGui::Button(ICON_FK_FLOPPY_O); // save button

            auto world = GetEdApp()->GetEditorWorld();
            bool isPlaying = world->GetPlaying();

            ImGui::BeginDisabled(isPlaying);
            if (ImGui::Button(ICON_FK_PLAY))
            {
                world->BeginPlay();
            }
            ImGui::EndDisabled();

            ImGui::BeginDisabled(!isPlaying);
            if (ImGui::Button(ICON_FK_STOP))
            {
                world->EndPlay();
            }
            ImGui::EndDisabled();

            ImGui::EndMenuBar();
        }
    }

} // namespace pulsared