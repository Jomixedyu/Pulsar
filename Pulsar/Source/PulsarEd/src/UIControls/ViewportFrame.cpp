#include <PulsarEd/UIControls/ViewportFrame.h>

#include "Pulsar/Components/CameraComponent.h"
#include <Pulsar/ImGuiImpl.h>
#include <Pulsar/Node.h>
#include "EditorWorld.h"
#include <ImGuiExt.h>

namespace pulsared
{
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
            cam = world->GetCurrentCamera();
        else
            cam = world->GetCameraManager().GetMainCamera();

        if (!cam)
        {
            ImGui::Text("No Camera");
            return false;
        }

        bool isResize = false;

        const auto contentSize = Vector2f { ImGuiExt::GetContentSize() };
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
        if (auto world = dynamic_cast<EditorWorld*>(m_world))
        {
            $$(world->GetTool())->Tick(dt);
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


} // namespace pulsared