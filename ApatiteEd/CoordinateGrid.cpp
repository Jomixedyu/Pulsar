#include "CoordinateGrid.h"
#include <ThirdParty/glad/glad.h>
#include <Apatite/Rendering/ShaderProgram.h>
#include <ApatiteEd/Windows/EditorWindowManager.h>

namespace apatiteed
{


    static const char* vertcode = R"___REGION__(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColor;

out vec4 vertexColor;

//uniform mat4 PROJECTION;
//uniform mat4 VIEW;
//uniform mat4 MODEL;


void main()
{
    //mat4 transform = PROJECTION * VIEW * MODEL;
    //gl_Position = transform * vec4(aPos, 1.0);
    gl_Position = vec4(aPos, 1.0);
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


    static ShaderProgram* program;
    static array_list<Vector3f> vert;
    static array_list<LinearColorf> colors;
    static uint32_t vao, vbo, colorvbo;

    void CoordinateGrid::Init()
    {

        int line_count = 11;
        float detail_distance = 1;
        float total_width = detail_distance * line_count;

        for (int x = -line_count / 2; x <= line_count / 2; x++)
        {
            vert.push_back({ 1 / (detail_distance * x), 1 / (total_width / 2) ,0 });
            vert.push_back({ 1 / (total_width / 2), 1 / (detail_distance * x), 0 });

            //vert.push_back({ 1 / (-detail_distance * x), 1 / (total_width / 2),  0 });
            colors.push_back({ 1,1,1,1 });
            colors.push_back({ 1,1,1,1 });
        }
        for (int z = -line_count / 2; z <= line_count / 2; z++)
        {
            //vert.push_back({ 1 / (total_width / 2), 1 / (detail_distance * z), 0 });
            vert.push_back({ 1 / (detail_distance * z), 1 / (total_width / 2) ,0 });
            vert.push_back({ 1 / (total_width / 2),1 / (-detail_distance * z), 0 });
            colors.push_back({ 1,1,0,1 });
            colors.push_back({ 1,1,0,1 });
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

        program = new ShaderProgram("test", vertcode, fragcode);
    }
    void CoordinateGrid::Render()
    {
        ShaderProgramRAII sp(program);

        glBindVertexArray(vao);
        auto scenewin = EditorWindowManager::GetWindow<SceneWindow>();
        //sp->SetUniformMatrix4fv("MODEL", Matrix4f::StaticScalar());
        //sp->SetUniformMatrix4fv("MODEL", scenewin->GetSceneCameraNode()->GetModelMatrix());
        //sp->SetUniformMatrix4fv("VIEW", scenewin->GetSceneCamera()->GetViewMat());
        //sp->SetUniformMatrix4fv("PROJECTION", scenewin->GetSceneCamera()->GetProjectionMat());
        //
        assert(scenewin && scenewin->get_is_opened());

        auto a = vert.size();
        glLineWidth(3);
        glDrawArrays(GL_LINES, 0, a);

        glBindVertexArray(0);
    }
}