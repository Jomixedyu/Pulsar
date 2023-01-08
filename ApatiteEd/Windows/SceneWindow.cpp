#include <ApatiteEd/Windows/SceneWindow.h>
#include <glad/glad.h>
#include <glfw/include/GLFW/glfw3.h>
#include <Apatite/Assets/Shader.h>
#include <Apatite/Scene.h>
#include <ApatiteEd/Assembly.h>
#include <ApatiteEd/EditorNode.h>
#include <Apatite/Components/CameraComponent.h>

namespace apatiteed
{
    //    static uint32_t fbo;
    //    static uint32_t tex;
    //    static uint32_t rbo;
    //    ShaderProgram* screenShader;
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
        EditorNode_sp node = mksptr(new EditorNode);
        node->set_name("EditorSceneCamera");
        auto cam = node->AddComponent<CameraComponent>();
        cam->cameraMode = CameraMode::Perspective;
        cam->backgroundColor = LinearColorf{ 0.2,0.2,0.2,1 }; 

        auto rt = mksptr(new RenderTexture);
        rt->PostInitialize(1024, 1024);
        rt->Construct();
        rt->BindGPU();
        cam->render_target = rt;

        World::Current()->scene->AddNode(node);

    }

    void SceneWindow::OnClose()
    {

    }

    void SceneWindow::OnDrawImGui()
    {

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
                        this->drawmode_select_index = i;
                        this->drawmode_select_index = 0;
                    }
                }
                ImGui::EndCombo();
            }
            ImGui::EndMenuBar();
        }

        auto cam = World::Current()->scene->get_root_nodes()->at(0)->GetComponent<CameraComponent>();

        cam->Render();
        
        ImVec2 vMin = ImGui::GetWindowContentRegionMin();
        ImVec2 vMax = ImGui::GetWindowContentRegionMax();

        vMin.x += ImGui::GetWindowPos().x;
        vMin.y += ImGui::GetWindowPos().y;
        vMax.x += ImGui::GetWindowPos().x;
        vMax.y += ImGui::GetWindowPos().y;

        //ImGui::GetForegroundDrawList()->AddRect(vMin, vMax, IM_COL32(255, 255, 0, 255));
        ImVec2 content_size = { vMax.x - vMin.x, vMax.y - vMin.y };
        
        ImGui::Image((ImTextureID)cam->render_target->get_tex_id(), content_size, ImVec2(0, 1), ImVec2(1, 0));

    }
}