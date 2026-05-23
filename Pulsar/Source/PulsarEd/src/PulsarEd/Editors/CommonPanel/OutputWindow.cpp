#include "Editors/CommonPanel/OutputWindow.h"

#include "PulsarEd/UIControls/ViewportFrame.h"
#include "PulsarEd/EditorWorld.h"
#include "PulsarEd/EditorAppInstance.h"
#include "PulsarEd/Editors/EditorWindow.h"
#include "PulsarEd/Editors/SceneEditor/SceneEditor.h"
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
        m_viewportFrame = std::make_unique<ViewportFrame>();
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
            m_viewportFrame.reset();
        }
    }

    void OutputWindow::OnDrawImGui(float dt)
    {
        auto sceneEditor = dynamic_cast<SceneEditor*>(GetParentEditorWindow()->GetEditor());

        if (ImGui::BeginMenuBar())
        {
            if (ImGui::Button(ICON_FK_ARROWS " Gizmos###OutputGizmos"))
            {
                auto world = sceneEditor ? sceneEditor->GetPreviewWorld() : nullptr;
                if (auto cam = world ? world->GetCameraManager().GetMainCamera() : nullptr)
                {
                    cam->SetGizmoPassEnabled(!cam->IsGizmoPassEnabled());
                }
            }
            ImGui::EndMenuBar();
        }

        if (m_viewportFrame)
        {
            auto world = sceneEditor ? sceneEditor->GetPreviewWorld() : nullptr;
            m_viewportFrame->SetWorld(world);
            m_viewportFrame->Render(dt);
        }
    }

    World* OutputWindow::RouteInput(const std::vector<uinput::InputEvent>& events)
    {
        auto* world = m_viewportFrame ? m_viewportFrame->GetWorld() : nullptr;
        if (!world)
            return nullptr;

        float vpX = 0.0f, vpY = 0.0f, vpW = 0.0f, vpH = 0.0f;
        bool focused = false;
        if (m_viewportFrame)
        {
            vpX = m_viewportFrame->GetLastViewportX();
            vpY = m_viewportFrame->GetLastViewportY();
            vpW = m_viewportFrame->GetLastViewportW();
            vpH = m_viewportFrame->GetLastViewportH();
            focused = m_viewportFrame->GetLastHasFocus();
        }

        for (auto e : events)
        {
            switch (e.type)
            {
            case uinput::InputEvent::KeyDown:
                if (focused)
                    world->ProcessInputEvent(e);
                break;
            case uinput::InputEvent::KeyUp:
                world->ProcessInputEvent(e); // always release to avoid stuck keys
                break;
            case uinput::InputEvent::MouseMove:
                if (e.mouseX >= vpX && e.mouseX < vpX + vpW &&
                    e.mouseY >= vpY && e.mouseY < vpY + vpH)
                {
                    e.mouseX -= vpX;
                    e.mouseY -= vpY;
                    world->ProcessInputEvent(e);
                }
                break;
            case uinput::InputEvent::MouseButtonDown:
                if (e.mouseX >= vpX && e.mouseX < vpX + vpW &&
                    e.mouseY >= vpY && e.mouseY < vpY + vpH)
                {
                    e.mouseX -= vpX;
                    e.mouseY -= vpY;
                    world->ProcessInputEvent(e);
                }
                break;
            case uinput::InputEvent::MouseButtonUp:
                world->ProcessInputEvent(e); // always release to avoid stuck buttons
                break;
            case uinput::InputEvent::MouseWheel:
                if (focused)
                    world->ProcessInputEvent(e);
                break;
            }
        }
        return world;
    }
}
