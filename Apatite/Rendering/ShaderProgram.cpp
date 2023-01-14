#include "ShaderProgram.h"
#include <ThirdParty/glad/glad.h>
#include <Apatite/Assets/Shader.h>

namespace apatite
{

    static bool _CheckShaderProgram(int id)
    {
        int isSuccess;
        glGetProgramiv(id, GL_LINK_STATUS, &isSuccess);
        return isSuccess;
    }
    static bool _CheckShaderCompile(const uint32_t& shaderId)
    {
        int isSuccess;
        glGetShaderiv(shaderId, GL_COMPILE_STATUS, &isSuccess);
        return isSuccess;
    }
    static string _GetShaderCompileErrorInfo(const uint32_t& shaderId)
    {
        char info[512];
        glGetShaderInfoLog(shaderId, 512, nullptr, info);
        return string(info);
    }

    ShaderProgram::ShaderProgram(const char* name, const char* vert_code, const char* frag_code)
    {
        this->program_id = glCreateProgram();

        this->vert_id = glCreateShader(GL_VERTEX_SHADER);
        this->frag_id = glCreateShader(GL_FRAGMENT_SHADER);

        glShaderSource(this->vert_id, 1, &vert_code, nullptr);
        glCompileShader(this->vert_id);

        if (!_CheckShaderCompile(this->vert_id)) {
            throw ShaderCompileException(name, _GetShaderCompileErrorInfo(this->vert_id));
        }

        glShaderSource(this->frag_id, 1, &frag_code, nullptr);
        glCompileShader(this->frag_id);
        if (!_CheckShaderCompile(this->frag_id)) {
            throw ShaderCompileException(name, _GetShaderCompileErrorInfo(this->frag_id));
        }

        glAttachShader(this->program_id, this->vert_id);
        glAttachShader(this->program_id, this->frag_id);
        glLinkProgram(this->program_id);

        glDeleteShader(this->vert_id);
        glDeleteShader(this->frag_id);

        if (!_CheckShaderProgram(this->program_id)) {
            int success;
            char infoLog[512];
            glGetProgramiv(this->program_id, GL_LINK_STATUS, &success);
            if (!success) {
                glGetProgramInfoLog(this->program_id, 512, NULL, infoLog);
                throw ShaderCompileException(name, infoLog);
            }
        }
    }

    int32_t ShaderProgram::GetUniformLocaltion(string_view name)
    {
        return glGetUniformLocation(this->program_id, name.data());
    }
    void ShaderProgram::SetUniformInt(string_view name, const int32_t& i)
    {
        glUniform1i(this->GetUniformLocaltion(name), i);
    }
    void ShaderProgram::SetUniformFloat(string_view name, const float& f)
    {
        glUniform1f(this->GetUniformLocaltion(name), f);
    }
    void ShaderProgram::SetUniformMatrix4fv(string_view name, const float* value)
    {
        glUniformMatrix4fv(this->GetUniformLocaltion(name), 1, GL_FALSE, value);
    }
    void ShaderProgram::SetUniformMatrix4fv(string_view name, const Matrix4f& mat)
    {
        SetUniformMatrix4fv(name, mat.get_value_ptr());
    }
    void ShaderProgram::SetUniformVector3(string_view name, const Vector3f& value)
    {
        glUniform3fv(this->GetUniformLocaltion(name), 1, value.get_value_ptr());
    }
    void ShaderProgram::SetUniformColor(string_view name, const LinearColorf& value)
    {
        glUniform4fv(this->GetUniformLocaltion(name), 1, value.get_value_ptr());
    }
    void ShaderProgram::SetUniformColor(string_view name, const Vector3f& value)
    {
        SetUniformColor(name, LinearColorf(value.x, value.y, value.z));
    }

    ShaderProgram::~ShaderProgram()
    {
        glDeleteProgram(this->program_id);
    }

    void ShaderProgram::EnableProgram()
    {
        glGetIntegerv(GL_CURRENT_PROGRAM, &this->last_program_id);
        glUseProgram(this->program_id);
    }
    void ShaderProgram::DisableProgram()
    {
        glUseProgram(this->last_program_id);
    }
}