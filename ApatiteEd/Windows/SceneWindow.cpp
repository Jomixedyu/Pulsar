#include <apatiteed/Windows/SceneWindow.h>
#include <glad/glad.h>
#include <glfw/include/GLFW/glfw3.h>


namespace apatiteed
{
//    static uint32_t fbo;
//    static uint32_t tex;
//    static uint32_t rbo;
//    ShaderProgram* screenShader;
//    static unsigned int quadVAO, quadVBO;
//
//    float quadVertices[] = {
//-1.0f,  1.0f,  0.0f, 1.0f,
//-1.0f, -1.0f,  0.0f, 0.0f,
// 1.0f, -1.0f,  1.0f, 0.0f,
//
//-1.0f,  1.0f,  0.0f, 1.0f,
// 1.0f, -1.0f,  1.0f, 0.0f,
// 1.0f,  1.0f,  1.0f, 1.0f
//    };

    SceneWindow::SceneWindow()
    {
        //glGenFramebuffers(1, &fbo);
        //glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        //glGenTextures(1, &tex);
        //glBindTexture(GL_TEXTURE_2D, tex);

        //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 800, 600, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        //glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

        ////glGenRenderbuffers(1, &rbo);
        ////glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        ////glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 800, 600);
        ////glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

        //this->window_flags = ImGuiWindowFlags_MenuBar;

        //string dataPath = Resource::GetLocalPath();
        //string texturePath = dataPath + "/texture";
        //string shaderPath = dataPath + "/shader";

        //Shader vert = Shader::CreateVetexShader("UnlitVert", FileUtil::ReadAllText(shaderPath + "/Unlit.vert"));
        //Shader frag = Shader::CreateFragmentShader("UnlitFrag", FileUtil::ReadAllText(shaderPath + "/Unlit.frag"));
        //screenShader = new ShaderProgram{ "Unlit" };
        //screenShader->AttachShader(vert);
        //screenShader->AttachShader(frag);
        //screenShader->Link();



        //glGenVertexArrays(1, &quadVAO);
        //glGenBuffers(1, &quadVBO);
        //glBindVertexArray(quadVAO);
        //glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        //glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        //glEnableVertexAttribArray(0);
        //glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        //glEnableVertexAttribArray(1);
        //glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

        //glBindFramebuffer(GL_FRAMEBUFFER, 0);
        //glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void SceneWindow::OnOpen()
    {
    }

    void SceneWindow::OnClose()
    {
    }

    void SceneWindow::OnDrawImGui()
    {

        if (ImGui::BeginMenuBar())
        {
            const char* items[] = { "Shade" };
            static int item_current_idx = 0;
            const char* combo_preview_value = items[item_current_idx];
            ImGui::SetNextItemWidth(150);
            if (ImGui::BeginCombo("Draw Mode", combo_preview_value))
            {

                ImGui::EndCombo();
            }
            ImGui::EndMenuBar();
        }


        //glBindFramebuffer(GL_FRAMEBUFFER, fbo);


        //pbr_render();
        //glBindFramebuffer(GL_FRAMEBUFFER, 0);
        //glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        //glClear(GL_COLOR_BUFFER_BIT);

        //screenShader->UseProgram();
        //glBindVertexArray(quadVAO);
        //glDisable(GL_DEPTH_TEST);
        //glBindTexture(GL_TEXTURE_2D, tex);
        //glDrawArrays(GL_TRIANGLES, 0, 6);

        //ImVec2 wsize = ImGui::GetWindowSize();
        //Screen::set_size({ wsize.x, wsize.y });
        //ImGui::Image((ImTextureID)tex, wsize, ImVec2(0, 1), ImVec2(1, 0));
    }
}