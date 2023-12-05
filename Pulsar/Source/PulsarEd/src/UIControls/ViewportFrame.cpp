#include <PulsarEd/UIControls/ViewportFrame.h>
#include <Pulsar/World.h>
#include <Pulsar/ImGuiImpl.h>
#include <Pulsar/Node.h>

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
    }

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

            // change new rt
            const auto newRt = RenderTexture::StaticCreate("BackBufferRT"_idxstr, (int)viewportSize->x, (int)viewportSize->y, true, true);
            if (const auto oldRt = cam->GetRenderTarget())
            {
                const auto oldClearColor = cam->GetRenderTarget()->GetGfxRenderTarget0()->ClearColor;
                DestroyObject(oldRt);

                newRt->GetGfxRenderTarget0()->ClearColor = oldClearColor;
            }

            cam->SetRenderTarget(newRt);
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
        ImGui::Image((void*)imgId, ImVec2(contentSize.x, contentSize.y), ImVec2(0, 1), ImVec2(1, 0));

        return isResize;
    }

    void ViewportFrame::Render(float dt)
    {
        PreviewFrame(m_world, true, &m_viewportSize, m_descriptorSet.get());
    }

    void ViewportFrame::Initialize()
    {
        m_descriptorLayout = Application::GetGfxApp()->CreateDescriptorSetLayout({
            gfx::GFXDescriptorSetLayoutInfo(
                gfx::GFXDescriptorType::CombinedImageSampler,
                gfx::GFXShaderStageFlags::Fragment,
                0)
            });
        m_descriptorSet = Application::GetGfxApp()->GetDescriptorManager()->GetDescriptorSet(m_descriptorLayout);
    }

    void ViewportFrame::Terminate()
    {
        m_descriptorSet.reset();
        m_descriptorLayout.reset();
    }

}