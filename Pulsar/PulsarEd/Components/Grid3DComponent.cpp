#include "Grid3DComponent.h"
#include <ThirdParty/glad/glad.h>
#include <Pulsar/Rendering/ShaderPass.h>
#include <PulsarEd/Windows/EditorWindowManager.h>
#include <PulsarEd/Windows/SceneWindow.h>

namespace pulsared
{

    static const char* vertcode = R"___REGION__(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColor;

out vec4 vertexColor;

uniform mat4 PROJECTION;
uniform mat4 VIEW;
uniform mat4 MODEL;


void main()
{
    mat4 transform = PROJECTION * VIEW * MODEL;
    gl_Position = transform * vec4(aPos, 1.0);
    //gl_Position = vec4(aPos, 1.0);
    vertexColor = aColor;

})___REGION__";

    static const char* fragcode = R"___REGION__(
#version 330 core
in vec4 vertexColor;
out vec4 FragColor;

void main()
{
    FragColor = vertexColor;
})___REGION__";

    static ShaderPass* program;
    static array_list<Vector3f> vert;
    static array_list<LinearColorf> colors;
    static uint32_t vao, vbo, colorvbo;

    void Grid3DComponent::OnInitialize()
    {
        int line_count = 20;
        float detail_distance = 1;
        float total_width = detail_distance * line_count;

        for (int x = -line_count / 2; x <= line_count / 2; x++)
        {
            vert.push_back({ total_width / 2, 0, detail_distance * x });
            vert.push_back({ -total_width / 2, 0 , detail_distance * x });

            LinearColorf color = { 0.1f, 0.1f, 0.1f, 1 };
            if (x == 0)
            {
                color.r = 0.9f;
            }
            colors.push_back(color);
            colors.push_back(color);
        }
        for (int z = -line_count / 2; z <= line_count / 2; z++)
        {
            vert.push_back({ detail_distance * z, 0, total_width / 2 });
            vert.push_back({ detail_distance * z, 0, -total_width / 2 });
            LinearColorf color = { 0.1f, 0.1f, 0.1f, 1 };
            if (z == 0)
            {
                color.r = 0.2f;
                color.g = 0.2f;
                color.b = 1.f;
            }
            colors.push_back(color);
            colors.push_back(color);
        }
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &colorvbo);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, vert.size() * sizeof(Vector3f), vert.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, colorvbo);
        glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(LinearColorf), colors.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);

        program = new ShaderPass("Editor/Grid3D", vertcode, fragcode);
    }
    void Grid3DComponent::OnDraw()
    {
        ShaderPassScope sp(program);

        glBindVertexArray(vao);
        auto scenewin = EditorWindowManager::GetWindow<SceneWindow>();
        sp->SetUniformMatrix4fv("MODEL", Matrix4f::StaticScalar());
        //sp->SetUniformMatrix4fv("MODEL", scenewin->GetSceneCameraNode()->GetModelMatrix());
        sp->SetUniformMatrix4fv("VIEW", scenewin->GetSceneCamera()->GetViewMat());
        sp->SetUniformMatrix4fv("PROJECTION", scenewin->GetSceneCamera()->GetProjectionMat());
        //
        assert(scenewin && scenewin->get_is_opened());

        auto a = vert.size();

        glDrawArrays(GL_LINES, 0, a);

        glBindVertexArray(0);
    }
}