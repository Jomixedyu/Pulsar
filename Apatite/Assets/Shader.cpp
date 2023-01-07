
#include <Apatite/Assets/Shader.h>
#include <ThirdParty/glad/glad.h>

#include <Apatite/Assets/Texture.h>
#include <CoreLib.Serialization/JsonSerializer.h>
#include <Apatite/Private/RenderInterface.h>

namespace apatite
{
    using namespace std;

    Shader::Shader()
    {
        this->id_ = 0;
    }

    Shader::~Shader()
    {
        this->UnBindGPU();

    }

    string Shader::ToString() const
    {
        return this->name_;
    }

    void Shader::UseShader()
    {
        assert(this->GetIsBindGPU());
        glUseProgram(this->id_);
    }

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
    string _GetShaderCompileErrorInfo(const uint32_t& shaderId)
    {
        char info[512];
        glGetShaderInfoLog(shaderId, 512, nullptr, info);
        return string(info);
    }

    int32_t Shader::GetUniformLocaltion(std::string_view name)
    {
        return glGetUniformLocation(this->id_, name.data());
    }

    void Shader::SetUniformInt(std::string_view name, const int32_t& i)
    {
        glUniform1i(this->GetUniformLocaltion(name), i);
    }

    void Shader::SetUniformFloat(std::string_view name, const float& f)
    {
        glUniform1f(this->GetUniformLocaltion(name), f);
    }

    void Shader::SetUniformMatrix4fv(std::string_view name, const float* value)
    {
        glUniformMatrix4fv(this->GetUniformLocaltion(name), 1, GL_FALSE, value);
    }

    void Shader::SetUniformMatrix4fv(std::string_view name, const Matrix4f& mat)
    {
        SetUniformMatrix4fv(name, mat.get_value_ptr());
    }

    void Shader::SetUniformVector3(std::string_view name, const Vector3f& value)
    {
        glUniform3fv(this->GetUniformLocaltion(name), 1, value.get_value_ptr());
    }

    void Shader::SetUniformColor(std::string_view name, const LinearColorf& value)
    {
        glUniform4fv(this->GetUniformLocaltion(name), 1, value.get_value_ptr());
    }

    void Shader::SetUniformColor(std::string_view name, const Vector3f& value)
    {
        SetUniformColor(name, LinearColorf(value.x, value.y, value.z));
    }

    void Shader::SetUniformTexture(std::string_view name, Texture_rsp tex)
    {
        assert(tex->GetIsBindGPU());
        this->SetUniformInt(name, tex->get_tex_id());
    }

    void Shader::BindGPU()
    {
        assert(!this->GetIsBindGPU());

        for (auto& pass : this->pass_)
        {
            pass.program = glCreateProgram();

            pass.vert = glCreateShader(GL_VERTEX_SHADER);
            pass.frag = glCreateShader(GL_FRAGMENT_SHADER);

            const char* vertcode = pass.config_.vert_code.c_str();
            glShaderSource(pass.vert, 1, &vertcode, nullptr);
            glCompileShader(pass.vert);

            if (!_CheckShaderCompile(pass.vert)) {
                throw ShaderCompileException(pass.config_.name, _GetShaderCompileErrorInfo(pass.vert));
            }

            const char* fragcode = pass.config_.frag_code.c_str();
            glShaderSource(pass.frag, 1, &fragcode, nullptr);
            glCompileShader(pass.frag);
            if (!_CheckShaderCompile(pass.frag)) {
                throw ShaderCompileException(pass.config_.name, _GetShaderCompileErrorInfo(pass.frag));
            }

            glAttachShader(pass.program, pass.vert);
            glAttachShader(pass.program, pass.frag);
            glLinkProgram(pass.program);

            if (!_CheckShaderProgram(pass.program)) {
                int success;
                char infoLog[512];
                glGetProgramiv(pass.program, GL_LINK_STATUS, &success);
                if (!success) {
                    glGetProgramInfoLog(pass.program, 512, NULL, infoLog);
                    throw ShaderCompileException(pass.config_.name, infoLog);
                }
            }
        }

        this->id_ = 1;
    }

    void Shader::UnBindGPU()
    {
        if (this->GetIsBindGPU())
        {
            glDeleteProgram(this->id_);
            this->id_ = 0;
        }
    }

    bool Shader::GetIsBindGPU()
    {
        return this->id_ != 0;
    }


    Shader_sp Shader::StaticCreate(string_view name, array_list<ShaderPass>&& pass)
    {
        auto shader = mksptr(new Shader);
        shader->Construct();
        shader->set_name(name);
        shader->pass_ = std::move(pass);
        return shader;
    }
}
