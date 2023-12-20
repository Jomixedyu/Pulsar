#include "Pulsar/Components/CameraComponent.h"

#include <Pulsar/ImGuiImpl.h>
#include <Pulsar/Node.h>
#include <Pulsar/World.h>
#include <PulsarEd/UIControls/ViewportFrame.h>

namespace pulsared
{
    namespace PImGui
    {
        Vector2f GetContentSize()
        {
            ImVec2 vMin = ImGui::GetWindowContentRegionMin();
            ImVec2 vMax = ImGui::GetWindowContentRegionMax();
            vMin.x += ImGui::GetWindowPos().x;
            vMin.y += ImGui::GetWindowPos().y;
            vMax.x += ImGui::GetWindowPos().x;
            vMax.y += ImGui::GetWindowPos().y;
            return {vMax.x - vMin.x, vMax.y - vMin.y};
        }
    } // namespace PImGui

    // ret: viewport Size Changed
    static bool PreviewFrame(World* world, bool isPreviewCam, Vector2f* viewportSize, gfx::GFXDescriptorSet* descriptorSet)
    {
        if (!world)
        {
            ImGui::Text("no world");
            return false;
        }

        CameraComponent_ref cam;
        if (isPreviewCam)
            cam = world->GetPreviewCamera();
        else
            cam = world->GetCameraManager().GetMainCamera();

        if (!cam)
        {
            ImGui::Text("No Camera");
            return false;
        }

        bool isResize = false;

        const auto contentSize = PImGui::GetContentSize();
        if (*viewportSize != contentSize)
        {
            isResize = true;
            *viewportSize = contentSize;

            cam->ResizeManagedRenderTexture((int)viewportSize->x, (int)viewportSize->y);
        }

        const auto descSet = descriptorSet;
        {
            auto desc = descSet->FindByBinding(0);
            if (!desc)
            {
                desc = descSet->AddDescriptor("p", 0);
            }
            desc->SetTextureSampler2D(cam->GetRenderTarget()->GetGfxRenderTarget0().get());
        }

        descSet->Submit();

        const auto imgId = descSet->GetId();
        ImGui::Image((void*)imgId, ImVec2(contentSize.x, contentSize.y));

        return isResize;
    }

    void ViewportFrame::Render(float dt)
    {
        PreviewFrame(m_world, true, &m_viewportSize, m_descriptorSet.get());

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
                    if(targetValue > 0)
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

    void ViewportFrame::Initialize()
    {
        m_descriptorLayout = Application::GetGfxApp()->CreateDescriptorSetLayout({gfx::GFXDescriptorSetLayoutInfo(
            gfx::GFXDescriptorType::CombinedImageSampler,
            gfx::GFXShaderStageFlags::Fragment,
            0)});
        m_descriptorSet = Application::GetGfxApp()->GetDescriptorManager()->GetDescriptorSet(m_descriptorLayout);
    }

    void ViewportFrame::Terminate()
    {
        m_descriptorSet.reset();
        m_descriptorLayout.reset();
    }

} // namespace pulsared