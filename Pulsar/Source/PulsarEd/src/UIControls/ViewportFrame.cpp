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
    static bool PreviewFrame(World* world, bool isPreviewCam, Vector2f* viewportSize, gfx::GFXDescriptorSet* descriptorSet, bool forceResize = false)
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
        if (*viewportSize != contentSize || forceResize)
        {
            isResize = true;
            viewportSize->x = (int)contentSize.x > 0 ? contentSize.x : 1;
            viewportSize->y = (int)contentSize.y > 0 ? contentSize.y : 1;

            cam->ResizeManagedRenderTexture((int)viewportSize->x, (int)viewportSize->y);
        }

        const auto descSet = descriptorSet;
        {
            auto desc = descSet->FindByBinding(0);
            if (!desc)
            {
                desc = descSet->AddDescriptor("p", 0);
            }
            desc->SetTextureSampler2D(cam->GetRenderTexture()->GetGfxRenderTarget0().get());
        }

        descSet->Submit();

        const auto imgId = descSet->GetId();
        ImGui::Image((void*)imgId, ImVec2(contentSize.x, contentSize.y));

        return isResize;
    }

    void ViewportFrame::SetWorld(World* world)
    {
        if (m_world == world)
        {
            return;
        }
        m_world = world;
        m_newWorld = true;
    }
    void ViewportFrame::Render(float dt)
    {
        PreviewFrame(m_world, true, &m_viewportSize, m_descriptorSet.get(), m_newWorld);
        m_newWorld = false;
        if (m_tool)
        {
            m_tool->Tick(dt);
        }
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
    void ViewportFrame::SetTool(std::unique_ptr<EdTool>&& tool)
    {
        if (m_tool)
        {
            m_tool->End();
        }
        m_tool = std::move(tool);
        if (m_tool)
        {
            m_tool->Initialize(m_world);
            m_tool->Begin();
        }
    }

} // namespace pulsared