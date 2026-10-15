#include <PulsarEd/UIControls/ViewportFrame.h>

#include "Pulsar/Components/CameraComponent.h"
#include <Pulsar/Application.h>
#include <Pulsar/ImGuiImpl.h>
#include <Pulsar/Node.h>
#include <Pulsar/Rendering/RenderThread.h>
#include "EditorWorld.h"
#include <ImGuiExt.h>
#include <gfx/GFXResourceRegistry.h>

namespace pulsared
{
    ViewportFrame::~ViewportFrame() = default;

    // ret: viewport Size Changed
    static bool PreviewFrame(World* world, bool isPreviewCam, Vector2f* viewportSize, bool forceResize = false)
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
            const float startX = ImGui::GetCursorPosX();
            const float startY = ImGui::GetCursorPosY();
            const auto avail = ImGui::GetContentRegionAvail();
            ImGui::SetWindowFontScale(3.0f);
            const auto textSize = ImGui::CalcTextSize("No Camera");
            ImGui::SetCursorPosX(startX + (avail.x - textSize.x) * 0.5f);
            ImGui::SetCursorPosY(startY + (avail.y - textSize.y) * 0.5f);
            ImGui::Text("No Camera");
            ImGui::SetWindowFontScale(1.0f);
            return false;
        }

        bool isResize = false;

        const auto contentSize = Vector2f { ImGuiExt::GetContentSize() };
        const auto& io = ImGui::GetIO();
        const Vector2f pixelSize = {
            contentSize.x * io.DisplayFramebufferScale.x,
            contentSize.y * io.DisplayFramebufferScale.y
        };

        if (*viewportSize != pixelSize || forceResize)
        {
            isResize = true;
            viewportSize->x = (int)pixelSize.x > 0 ? pixelSize.x : 1;
            viewportSize->y = (int)pixelSize.y > 0 ? pixelSize.y : 1;

            cam->ResizeManagedRenderTexture((int)viewportSize->x, (int)viewportSize->y);
        }

        auto rt = cam->GetRenderTexture();
        uint64_t imgId = 0;
        if (rt)
        {
            auto view = rt->GetGfxColorTextureView();
            if (view && view->GetTexture())
                imgId = view->GetTexture()->GetNativeViewHandle();
        }
        if (!imgId)
            return false;   // render texture not created on the render thread yet

        ImGui::Image((ImTextureID)imgId, ImVec2(contentSize.x, contentSize.y));

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
        // Capture viewport geometry for the caller (e.g. SceneWindow) to sync to InputContext
        m_lastViewportX = ImGui::GetCursorScreenPos().x;
        m_lastViewportY = ImGui::GetCursorScreenPos().y;
        m_lastViewportW = ImGui::GetContentRegionAvail().x;
        m_lastViewportH = ImGui::GetContentRegionAvail().y;
        m_lastHasFocus = ImGui::IsWindowHovered() && ImGui::IsWindowFocused();

        PreviewFrame(m_world, m_isPreviewCam, &m_viewportSize, m_newWorld);
        m_newWorld = false;
        if (m_enableEdToolTick)
        {
            if (auto world = dynamic_cast<EditorWorld*>(m_world))
            {
                $$(world->GetTool())->Tick(dt);
            }
        }
    }

    void ViewportFrame::Initialize()
    {
    }

    void ViewportFrame::Terminate()
    {
    }


} // namespace pulsared
