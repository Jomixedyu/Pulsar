#include "Editors/CommonPanel/OutputWindow.h"

#include "PulsarEd/UIControls/ViewportFrame.h"
#include "PulsarEd/EditorWorld.h"
#include "PulsarEd/EditorAppInstance.h"
#include "PulsarEd/Workspace.h"

namespace pulsared
{
    void OutputWindow::OnOpen()
    {
        base::OnOpen();
        Workspace::OnWorkspaceOpened.AddListener(this, &ThisClass::OnOpenWorkspace);
        Workspace::OnWorkspaceClosed.AddListener(this, &ThisClass::OnCloseWorkspace);
        if (Workspace::IsOpened())
        {
            this->OnOpenWorkspace();
        }
    }

    void OutputWindow::OnClose()
    {
        base::OnClose();
        Workspace::OnWorkspaceOpened.RemoveListener(this, &ThisClass::OnOpenWorkspace);
        Workspace::OnWorkspaceClosed.RemoveListener(this, &ThisClass::OnCloseWorkspace);
        if (Workspace::IsOpened())
        {
            this->OnCloseWorkspace();
        }
    }

    void OutputWindow::OnOpenWorkspace()
    {
        m_viewportFrame = new ViewportFrame;
        m_viewportFrame->Initialize();
        m_viewportFrame->SetIsPreviewCamera(false);
        m_viewportFrame->SetEnableEdToolTick(false);
        m_viewportFrame->SetWorld(GetEdApp()->GetEditorWorld());
    }

    void OutputWindow::OnCloseWorkspace()
    {
        if (m_viewportFrame)
        {
            m_viewportFrame->Terminate();
            delete m_viewportFrame;
            m_viewportFrame = nullptr;
        }
    }

    void OutputWindow::OnDrawImGui(float dt)
    {
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::Button(ICON_FK_ARROWS " Gizmos###OutputGizmos"))
            {
                if (auto cam = EditorWorld::GetPreviewWorld()->GetCameraManager().GetMainCamera())
                {
                    cam->SetGizmoOverlayEnabled(!cam->IsGizmoOverlayEnabled());
                }
            }
            ImGui::EndMenuBar();
        }

        if (m_viewportFrame)
        {
            auto world = EditorWorld::GetPreviewWorld();
            m_viewportFrame->SetWorld(world);
            m_viewportFrame->Render(dt);
        }
    }
}
