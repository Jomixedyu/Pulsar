#include "..\RenderInterface.h"
#include "..\RenderInterface.h"
#include <Pulsar/Private/RenderInterface.h>
#include <ThirdParty/glad/glad.h>
#include <Pulsar/Assets/Texture2D.h>
#include <ThirdParty/glfw/include/GLFW/glfw3.h>
#include <Pulsar/Assets/Shader.h>

namespace pulsar::detail
{
    extern GLFWwindow* g_glfw_window_instance;

    namespace RenderInterface
    {

        using namespace std;

        void Clear(float r, float g, float b, float a)
        {
            glClearColor(r, g, b, a);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }

        void EnableDepthTest()
        {
            glEnable(GL_DEPTH_TEST);
        }
        void DisableDepthTest()
        {
            glDisable(GL_DEPTH_TEST);
        }

        void GetDefaultBufferViewport(int32_t* width, int32_t* height)
        {
            glfwGetFramebufferSize(g_glfw_window_instance, width, height);
        }
        void GetViewport(int32_t* width, int32_t* height)
        {
            
        }
        void SetViewport(int32_t x, int32_t y, int32_t width, int32_t height)
        {
            glViewport(x, y, width, height);
        }

        void LoadTexture2D(uint32_t channel_count, int32_t width, int32_t height, uint8_t* data, uint32_t* out_id)
        {
            glGenTextures(1, out_id);
            glBindTexture(GL_TEXTURE_2D, *out_id);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            auto img_type = GL_RGB;
            switch (channel_count)
            {
            case 3:
                img_type = GL_RGB;
                break;
            case 4:
                img_type = GL_RGBA;
                break;
            case 1:
                img_type = GL_RED;
                break;
            default:
                break;
            }

            glTexImage2D(GL_TEXTURE_2D, 0, img_type, width, height, 0, img_type, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);

            glBindTexture(GL_TEXTURE_2D, 0);

        }
        void UnloadTexture2D(uint32_t id)
        {
            glDeleteTextures(1, &id);
        }

        void UnloadTexture2Ds(uint32_t id[], int32_t length)
        {
            glDeleteTextures(length, id);
        }

        static uint32_t _frameCount = 0;
        static double lastTime = 0;
        static float deltaTime = 0;

        double GetTime()
        {
            return glfwGetTime();
        }

        float GetDeltaTime()
        {
            return deltaTime;
        }

        uint32_t GetFrameCount()
        {
            return _frameCount;
        }

        void Render()
        {
            glfwSwapBuffers(g_glfw_window_instance);
            _frameCount++;

            double curTime = GetTime();
            deltaTime = curTime - lastTime;
            lastTime = curTime;
        }

        string _GetShaderCompileErrorInfo(const uint32_t& shaderId)
        {
            char info[512];
            glGetShaderInfoLog(shaderId, 512, nullptr, info);
            return string(info);
        }
        static bool _CheckShaderCompile(const uint32_t& shaderId)
        {
            int isSuccess;
            glGetShaderiv(shaderId, GL_COMPILE_STATUS, &isSuccess);
            return isSuccess;
        }
        static bool _CreateShader(std::string_view shaderName, const uint32_t& shaderType, std::string_view shaderSource, uint32_t* out_id)
        {
            uint32_t shader;
            shader = glCreateShader(shaderType);

            const char* _srcStr = shaderSource.data();

            glShaderSource(shader, 1, &_srcStr, nullptr);
            glCompileShader(shader);

            if (!_CheckShaderCompile(shader)) {

                return false;
            }

            *out_id = shader;

            return true;
        }

        bool CompileVertexShader(std::string_view name, string_view code, uint32_t* out_id)
        {
            return _CreateShader(name, GL_VERTEX_SHADER, code, out_id);
        }
        bool CompileFragmentShader(std::string_view name, string_view code, uint32_t* out_id)
        {
            return _CreateShader(name, GL_FRAGMENT_SHADER, code, out_id);
        }
        void DeleteShader(uint32_t id)
        {
            glDeleteShader(id);
        }
        std::string GetShaderCompilerError(uint32_t id)
        {
            return _GetShaderCompileErrorInfo(id);
        }
    }
}