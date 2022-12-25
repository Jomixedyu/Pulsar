
#include <Apatite/Assets/Shader.h>
#include <ThirdParty/glad/glad.h>

#include <Apatite/Assets/Texture2D.h>
#include <CoreLib.Serialization/JsonSerializer.h>
#include <Apatite/Private/RenderInterface.h>

namespace apatite
{
    using namespace std;

    Shader::Shader()
    {
        //this->id_ = glCreateProgram();
    }

    Shader::~Shader()
    {
        glDeleteProgram(this->id_);
    }

    string Shader::ToString() const
    {
        return this->name_;
    }

    static bool _CheckShaderProgram(int id)
    {
        int isSuccess;
        glGetProgramiv(id, GL_LINK_STATUS, &isSuccess);
        return isSuccess;
    }

    void Shader::UseProgram()
    {
        if (this->get_isused())
        {
            return;
        }
        glUseProgram(this->id_);
        current_use_id = this->id_;
    }

    void Shader::AttachShader(const Shader& shaderId)
    {
        glAttachShader(this->id_, shaderId.get_id());
    }

    void Shader::Link()
    {
        glLinkProgram(this->id_);
        if (!_CheckShaderProgram(this->id_)) {
            int success;
            char infoLog[512];
            glGetProgramiv(this->id_, GL_LINK_STATUS, &success);
            if (!success) {
                glGetProgramInfoLog(this->id_, 512, NULL, infoLog);
                throw ShaderCompileException(this->name_, this->ToString() + infoLog);

            }
        }
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
        //Texture2D* tex = nullptr;
        //int i = 0;
        //for (auto& item : this->textures_)
        //{
        //    if (item->get_name() == tex_name)
        //    {
        //        tex = item;
        //        break;
        //    }
        //    ++i;
        //}
        //if (tex == nullptr)
        //{
        //    throw 0;
        //}
        //this->GetUniformLocaltion(name);
        //this->SetUniformInt(name, i); //set sampler
        //glActiveTexture(GL_TEXTURE0 + i);
        //glBindTexture(GL_TEXTURE_2D, tex->get_id());
    }

}
