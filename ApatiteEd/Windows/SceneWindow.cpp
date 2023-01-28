#include <ApatiteEd/Windows/SceneWindow.h>
#include <glad/glad.h>
#include <glfw/include/GLFW/glfw3.h>
#include <Apatite/Assets/Shader.h>
#include <Apatite/Scene.h>
#include <ApatiteEd/Assembly.h>
#include <ApatiteEd/EditorNode.h>
#include <Apatite/Components/CameraComponent.h>
#include <ApatiteEd/Importers/FBXImporter.h>
#include <ApatiteEd/Components/StdEditCameraControllerComponent.h>
#include <ApatiteEd/Components/Grid3DComponent.h>
#include <Apatite/Rendering/RenderContext.h>

namespace apatiteed
{
    //    static uint32_t fbo;
    //    static uint32_t tex;
    //    static uint32_t rbo;
    //    ShaderPass* screenShader;
    //    static unsigned int quadVAO, quadVBO;
    //
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
    static uint32_t fbo;
    static uint32_t tex;
    static uint32_t rbo;
    static uint32_t quadVAO;
    static uint32_t quadVBO;

    static Shader_sp default_shader;

    void SceneWindow::OnOpen()
    {
        EditorNode_sp center_node = EditorNode::StaticCreate("EditorCameraController");
        EditorNode_sp camera_node = EditorNode::StaticCreate("EditorCamera", center_node);
        this->camera_node = camera_node;
        this->camera_ctrl_node = center_node;

        auto cam = camera_node->AddComponent<CameraComponent>();

        cam->cameraMode = CameraMode::Perspective;
        cam->backgroundColor = LinearColorf{ 0.33,0.33,0.33,1 };
        cam->fov = 45.f;
        cam->near = 0.01f;
        cam->far = 10000.f;
        cam->size_ = { 1280,720 };
        camera_node->set_self_position({ 10.f, 7, 10 });
        //camera_node->set_self_euler_rotation({ -25.f, -45, 0 });

        center_node->AddComponent<StdEditCameraControllerComponent>();

        auto rt = mksptr(new RenderTexture);
        rt->PostInitializeData(1280, 720);
        rt->Construct();
        rt->BindGPU();
        cam->render_target = rt;


        //node->set_self_euler_rotation({ 0,-90,0 });
        World::Current()->scene->AddNode(center_node);

        EditorNode_sp grid3d = EditorNode::StaticCreate("Grid3d");
        grid3d->AddComponent<Grid3DComponent>();
        World::Current()->scene->AddNode(grid3d);
        Node_sp fbx = FBXImporter::Import(R"(C:/Users/JomiXedYu/Desktop/sphere.fbx)");
        World::Current()->scene->AddNode(fbx);
        
    }

    void SceneWindow::OnClose()
    {

    }

    void SceneWindow::OnDrawImGui()
    {
        RenderContext::PushCamera(this->GetSceneCamera());

        if (ImGui::BeginMenuBar())
        {
            const char* items[] = { "Default", "Shade", "Wire", "Unlit" };
            ImGui::SetNextItemWidth(150);
            if (ImGui::BeginCombo("Draw Mode", items[this->drawmode_select_index]))
            {
                for (size_t i = 0; i < 4; i++)
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

            ImGui::Button("Gizmos");

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

        if (this->win_size_.x != content_size_x || this->win_size_.y != content_size_y)
        {
            this->win_size_ = { content_size_x, content_size_y };
            this->OnWindowResize();
        }

        auto cam = this->GetSceneCamera();
        cam->size_ = this->win_size_;
        cam->Render();

        ImGui::Image((ImTextureID)cam->render_target->get_tex_id(), ImVec2(content_size_x, content_size_y), ImVec2(0, 1), ImVec2(1, 0));

        RenderContext::PopCamera();
    }

    void SceneWindow::OnWindowResize()
    {
        auto cam = this->GetSceneCamera();

        assert(cam);
        assert(cam->render_target);

        cam->render_target->UnBindGPU();
        cam->render_target = nullptr;

        auto rt = mksptr(new RenderTexture);
        rt->PostInitializeData(this->win_size_.x, this->win_size_.y);
        rt->Construct();
        rt->BindGPU();
        cam->render_target = rt;
    }

}