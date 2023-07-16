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

namespace pulsared
{

    static float quadVertices[] = {
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };
    static const char* vertcode = R"___REGION__(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

void main()
{
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
    TexCoords = aTexCoords;
})___REGION__";

    static const char* fragcode = R"___REGION__(
#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;

void main()
{
    FragColor = texture(screenTexture, TexCoords);
})___REGION__";

    SceneWindow::SceneWindow()
    {

    }

    static Shader_sp default_shader;

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
        World::Current()->scene->AddNode(camCtrlNode);

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

        //if (ImGui::BeginMenuBar())
        //{
        //    const char* items[] = { "Default", "Shade", "Wire", "Unlit" };
        //    ImGui::SetNextItemWidth(150);
        //    if (ImGui::BeginCombo("Draw Mode", items[this->drawmode_select_index]))
        //    {
        //        for (size_t i = 0; i < 4; i++)
        //        {
        //            bool selected = this->drawmode_select_index == i;
        //            if (ImGui::Selectable(items[i], selected))
        //            {
        //                this->drawmode_select_index = static_cast<int>(i);
        //                this->drawmode_select_index = 0;
        //            }
        //        }
        //        ImGui::EndCombo();
        //    }

        //    ImGui::Button("Gizmos");

        //    ImGui::EndMenuBar();
        //}

        //ImVec2 vMin = ImGui::GetWindowContentRegionMin();
        //ImVec2 vMax = ImGui::GetWindowContentRegionMax();

        //vMin.x += ImGui::GetWindowPos().x;
        //vMin.y += ImGui::GetWindowPos().y;
        //vMax.x += ImGui::GetWindowPos().x;
        //vMax.y += ImGui::GetWindowPos().y;
        //int content_size_x = vMax.x - vMin.x;
        //int content_size_y = vMax.y - vMin.y;

        //if (this->win_size_.x != content_size_x || this->win_size_.y != content_size_y)
        //{
        //    this->win_size_ = { content_size_x, content_size_y };
        //    this->OnWindowResize();
        //}

        //auto cam = this->GetSceneCamera();
        //cam->size_ = this->win_size_;
        //cam->Render();

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