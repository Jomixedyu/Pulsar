
#include "ViewEdTool.h"

#include "Pulsar/Components/CameraComponent.h"
#include "Pulsar/Node.h"
#include "imgui/imgui.h"

namespace pulsared
{
    void ViewEdTool::Tick(float dt)
    {
        base::Tick(dt);

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

        if (m_altPressed)
        {
            if (m_leftMousePressed && m_enabledRotate)
            {
                auto trans = m_world->GetPreviewCamera()->GetAttachedNode()->GetTransform()->GetParent();
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
                auto cam = m_world->GetPreviewCamera();
                auto tr = cam->GetAttachedNode()->GetTransform();
                auto dtDistance = (newpos.x - m_latestMousePos.x) * m_scaleSpeed;
                if (cam->GetProjectionMode() == CameraProjectionMode::Perspective)
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
                auto tr = m_world->GetPreviewCamera()->GetAttachedNode()->GetTransform()->GetParent();
                auto dtX = newpos.x - m_latestMousePos.x;
                auto dtY = newpos.y - m_latestMousePos.y;

                tr->Translate(tr->GetRight() * -dtX * 0.1f);
                tr->Translate(tr->GetUp() * dtY * 0.1f);
            }
        }
        m_latestMousePos = newpos;
    }
    void ViewEdTool::Begin()
    {
        base::Begin();

    }
} // namespace pulsared