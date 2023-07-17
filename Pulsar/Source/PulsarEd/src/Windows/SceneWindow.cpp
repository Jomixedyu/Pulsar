#include <PulsarEd/Windows/SceneWindow.h>
#include <ThirdParty/glfw/include/GLFW/glfw3.h>
#include <Pulsar/Assets/Shader.h>
#include <Pulsar/Scene.h>
#include <PulsarEd/Assembly.h>
#include <PulsarEd/EditorNode.h>
#include <Pulsar/Components/CameraComponent.h>
#include <PulsarEd/Importers/FBXImporter.h>
#include <PulsarEd/Components/StdEditCameraControllerComponent.h>
#include <PulsarEd/Components/Grid3DComponent.h>
#include <Pulsar/Rendering/RenderContext.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_vulkan.h>
#include <gfx-vk/GFXVulkanRenderTarget.h>
namespace pulsared
{

    SceneWindow::SceneWindow()
    {

    }

    void SceneWindow::OnOpen()
    {
        EditorNode_sp camCtrlNode = EditorNode::StaticCreate("EdCameraController");
        EditorNode_sp camNode = EditorNode::StaticCreate("EdCamera", camCtrlNode);
        this->m_camNode = camNode;
        this->m_camCtrlNode = camCtrlNode;

        auto cam = camNode->AddComponent<CameraComponent>();

        cam->cameraMode = CameraMode::Perspective;
        cam->backgroundColor = LinearColorf{ 0.33,0.33,0.33,1 };
        cam->fov = 45.f;
        cam->near = 0.01f;
        cam->far = 10000.f;
        cam->size_ = { 1280,720 };
        camNode->set_self_position({ 0.f, 7, 30 });
        ////camera_node->set_self_euler_rotation({ -25.f, -45, 0 });

        camCtrlNode->AddComponent<StdEditCameraControllerComponent>();

        auto rt = RenderTexture::StaticCreate(1280, 720, true, true);
        cam->SetRenderTarget(rt);

        ////node->set_self_euler_rotation({ 0,-90,0 });
        World::Current()->GetScene()->AddNode(camCtrlNode);

        //EditorNode_sp grid3d = EditorNode::StaticCreate("Grid3d");
        //grid3d->AddComponent<Grid3DComponent>();
        //World::Current()->scene->AddNode(grid3d);

    }

    void SceneWindow::OnClose()
    {

    }

    void SceneWindow::OnDrawImGui()
    {
        static bool demowin = true;
        if (demowin)
            ImGui::ShowDemoWindow(&demowin);
        //RenderContext::PushCamera(this->GetSceneCamera());

        if (ImGui::BeginMenuBar())
        {
            const char* items[] = { "Shade", "Wire", "Unlit" };

            ImGui::Text("Draw Mode");

            ImGui::SetNextItemWidth(150);
            if (ImGui::BeginCombo("##Draw Mode", items[this->drawmode_select_index]))
            {
                for (size_t i = 0; i < 3; i++)
                {
                    bool selected = this->drawmode_select_index == i;
                    if (ImGui::Selectable(items[i], selected))
                    {
                        this->drawmode_select_index = static_cast<int>(i);
                        this->drawmode_select_index = 0;
                    }
                }
                ImGui::EndCombo();
            }

            const char* editMode[] = { "SceneEditor", "Modeling" };
            ImGui::Text("Edit Mode");
            ImGui::SetNextItemWidth(150);
            if (ImGui::BeginCombo("##Edit Mode", editMode[this->m_editModeIndex]))
            {
                for (size_t i = 0; i < 2; i++)
                {
                    bool selected = this->m_editModeIndex == i;
                    if (ImGui::Selectable(editMode[i], selected))
                    {
                        this->m_editModeIndex = static_cast<int>(i);
                        this->m_editModeIndex = 0;
                    }
                }
                ImGui::EndCombo();
            }

            ImGui::Button(ICON_FK_ARROWS " Gizmos###Gizmos");
            ImGui::Button(ICON_FK_CUBE " 2D###2D");
            ImGui::Button(ICON_FK_TABLE " Grid###Grid");

            ImGui::EndMenuBar();
        }

        ImVec2 vMin = ImGui::GetWindowContentRegionMin();
        ImVec2 vMax = ImGui::GetWindowContentRegionMax();



        vMin.x += ImGui::GetWindowPos().x;
        vMin.y += ImGui::GetWindowPos().y;
        vMax.x += ImGui::GetWindowPos().x;
        vMax.y += ImGui::GetWindowPos().y;
        int content_size_x = vMax.x - vMin.x;
        int content_size_y = vMax.y - vMin.y;


        static VkDescriptorSet vkset;
        auto cam = this->GetSceneCamera();
        cam->size_ = m_viewportSize;

        if (this->m_viewportSize.x != content_size_x || this->m_viewportSize.y != content_size_y)
        {
            this->m_viewportSize = { content_size_x, content_size_y };
            this->OnWindowResize();
            auto rt = cam->GetRenderTarget()->GetGfxRenderTarget0();
            auto vkrt = static_cast<gfx::GFXVulkanRenderTarget*>(rt.get());
            vkset = ImGui_ImplVulkan_AddTexture(
                vkrt->GetVulkanTexture2d()->GetVkSampler(),
                vkrt->GetVulkanTexture2d()->GetVkImageView(),
                vkrt->GetVulkanTexture2d()->GetVkImageLayout());
        }

        
        cam->Render();

        //GetSceneCamera()->GetRenderTarget()->;
        ImGui::Image(&vkset, ImVec2(content_size_x, content_size_y), ImVec2(0, 1), ImVec2(1, 0));
        // 
        //ImGui::Image((ImTextureID)cam->render_target->get_tex_id(), ImVec2(content_size_x, content_size_y), ImVec2(0, 1), ImVec2(1, 0));

        //RenderContext::PopCamera();
    }

    void SceneWindow::OnWindowResize()
    {
        auto cam = this->GetSceneCamera();

        assert(cam);
        assert(cam->GetRenderTarget());

        cam->SetRenderTarget(RenderTexture::StaticCreate(m_viewportSize.x, m_viewportSize.y, true, true));
    }

}