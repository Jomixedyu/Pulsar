
#include "ViewEdTool.h"

#include "Components/StdEditCameraControllerComponent.h"
#include "EditorWorld.h"
#include "ImGuiExt.h"
#include "Pulsar/Components/CameraComponent.h"
#include "Pulsar/Node.h"
#include "imgui/imgui.h"

namespace pulsared
{
    static bool IsModifilerKeysDown()
    {
        return
        ImGui::IsKeyDown(ImGuiKey_LeftAlt)
        || ImGui::IsKeyDown(ImGuiKey_RightAlt)
        || ImGui::IsKeyDown(ImGuiKey_LeftShift)
        || ImGui::IsKeyDown(ImGuiKey_RightShift)
        || ImGui::IsKeyDown(ImGuiKey_LeftCtrl)
        || ImGui::IsKeyDown(ImGuiKey_RightCtrl);
    }

    void ViewEdTool::Tick(float dt)
    {
        base::Tick(dt);
        if (!m_world->GetCurrentCamera())
        {
            return;
        }

        auto newpos = ImGui::GetMousePos();
        if (ImGui::IsKeyDown(ImGuiKey_LeftAlt) && ImGui::IsWindowHovered())
        {
            m_altPressed = true;
        }
        if (ImGui::IsKeyReleased(ImGuiKey_LeftAlt))
        {
            m_altPressed = false;
        }
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && ImGui::IsWindowHovered())
        {
            m_leftMousePressed = true;
            m_latestMousePos = ImGui::GetMousePos();
        }
        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
        {
            m_leftMousePressed = false;
        }
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Middle) && ImGui::IsWindowHovered())
        {
            m_middleMousePressed = true;
            m_latestMousePos = ImGui::GetMousePos();
        }
        if (ImGui::IsMouseReleased(ImGuiMouseButton_Middle))
        {
            m_middleMousePressed = false;
        }
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) && ImGui::IsWindowHovered())
        {
            m_rightMousePressed = true;
            m_latestMousePos = ImGui::GetMousePos();
        }
        if (ImGui::IsMouseReleased(ImGuiMouseButton_Right))
        {
            m_rightMousePressed = false;
        }

        if (!IsModifilerKeysDown())
        {
            MouseEventData e{};
            e.Position = newpos;
            ImVec2 inRegionPos{};
            e.InRegion = ImGuiExt::GetMousePosOnContentRegion(inRegionPos);
            e.InRegionPosition = inRegionPos;

            for (int i = 0; i < ImGuiMouseButton_COUNT; ++i)
            {
                e.ButtonId = i;
                if (ImGui::IsMouseClicked(i))
                {
                    this->OnMouseDown(e);
                }
                if (ImGui::IsMouseReleased(i))
                {
                    this->OnMouseUp(e);
                }
            }
        }

        if (m_altPressed)
        {
            auto parent = m_world->GetCurrentCamera()->GetTransform()->GetParent()->GetNode();
            auto camCtrl = parent->GetComponent<StdEditCameraControllerComponent>();

            if (m_leftMousePressed && camCtrl->CanRotate())
            {
                auto trans = m_world->GetCurrentCamera()->GetNode()->GetTransform()->GetParent();
                auto euler = trans->GetEuler();
                Vector3f mouseDt{newpos.y - m_latestMousePos.y, newpos.x - m_latestMousePos.x, 0};
                if (euler.x + mouseDt.x < -85.f ||
                    euler.x + mouseDt.x > 85.f)
                {
                    mouseDt.x = 0;
                }
                trans->SetEuler(euler + mouseDt);
            }
            else if (m_rightMousePressed)
            {
                auto cam = m_world->GetCurrentCamera();
                auto tr = cam->GetNode()->GetTransform();
                auto dtDistance = (newpos.x - m_latestMousePos.x) * m_scaleSpeed;
                if (cam->GetProjectionMode() == CaptureProjectionMode::Perspective)
                {
                    if (tr->GetPosition().z + dtDistance > -0.2f)
                    {
                        // nothing
                    }
                    else
                    {
                        tr->Translate({0.f, 0, dtDistance});
                    }
                }
                else
                {
                    const auto targetValue = cam->GetOrthoSize() - dtDistance;
                    if (targetValue > 0)
                    {
                        cam->SetOrthoSize(targetValue);
                    }
                }
            }
            else if (m_middleMousePressed)
            {
                auto tr = m_world->GetCurrentCamera()->GetNode()->GetTransform()->GetParent();
                auto dtX = newpos.x - m_latestMousePos.x;
                auto dtY = newpos.y - m_latestMousePos.y;

                tr->Translate(tr->GetRight() * -dtX * 0.1f);
                tr->Translate(tr->GetUp() * dtY * 0.1f);
            }
        }

        auto& io = ImGui::GetIO();
        if (ImGui::IsWindowHovered())
        {
            if (io.MouseWheel != 0)
            {
                auto cam = m_world->GetCurrentCamera();
                auto tr = cam->GetNode()->GetTransform();
                auto dtDistance = io.MouseWheel * m_scaleSpeed * 10;
                if (cam->GetProjectionMode() == CaptureProjectionMode::Perspective)
                {
                    if (tr->GetPosition().z + dtDistance > -0.2f)
                    {
                        // nothing
                    }
                    else
                    {
                        tr->Translate({0.f, 0, dtDistance});
                    }
                }
                else
                {
                    const auto targetValue = cam->GetOrthoSize() - dtDistance;
                    if (targetValue > 0)
                    {
                        cam->SetOrthoSize(targetValue);
                    }
                }
            }

        }

        m_latestMousePos = newpos;


    }
    void ViewEdTool::Begin()
    {
        base::Begin();
    }
    void ViewEdTool::OnMouseDown(const MouseEventData& e)
    {
    }
    void ViewEdTool::OnMouseUp(const MouseEventData& e)
    {
    }

    void ViewEdTool::OnDragUpdate(const MouseEventData& e)
    {
    }
    void ViewEdTool::GetViewportSize(Vector2f& pos, Vector2f& size) const
    {
        ImVec2 vMin = ImGui::GetWindowContentRegionMin();
        ImVec2 vMax = ImGui::GetWindowContentRegionMax();
        vMin.x += ImGui::GetWindowPos().x;
        vMin.y += ImGui::GetWindowPos().y;
        vMax.x += ImGui::GetWindowPos().x;
        vMax.y += ImGui::GetWindowPos().y;

        ImVec2 vSize = { vMax.x - vMin.x, vMax.y - vMin.y };

        pos = vMin;
        size = vSize;
    }
} // namespace pulsared