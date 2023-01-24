
#include <Apatite/Assets/Shader.h>
#include <ThirdParty/glad/glad.h>

#include <Apatite/Assets/Texture.h>
#include <CoreLib.Serialization/JsonSerializer.h>
#include <Apatite/Private/RenderInterface.h>
#include <Apatite/Logger.h>

namespace apatite
{
    using namespace std;

    struct _ShaderInfo
    {
        string shader_name;
        string shader_config;
        array_list<string> pass_configs;
        array_list<string> pass_verts;
        array_list<string> pass_frags;
    };

    // temp impl
    static _ShaderInfo _ParseShaderInfo(const string& shaderbuilder)
    {
        auto arr = StringUtil::Split(shaderbuilder, u8char("\n"));
        _ShaderInfo info;
        int step = 0;

        constexpr int kShaderConfig = 1;
        constexpr int kPass = 2;
        constexpr int kVert = 3;
        constexpr int kFrag = 4;

        for (size_t i = 0; i < arr.size(); i++)
        {
            if (arr[i].starts_with("#pragma config"))
            {
                step = kShaderConfig;
                continue;
            }
            if (arr[i].starts_with("#pragma pass"))
            {
                step = kPass;
                continue;
            }
            if (arr[i].starts_with("#pragma vert"))
            {
                step = kVert;
                continue;
            }
            if (arr[i].starts_with("#pragma frag"))
            {
                step = kFrag;
                continue;
            }
            
            if (step == kShaderConfig)
            {
                info.shader_config.append(std::move(arr[i]));
            }
            if (step == kPass)
            {
                // new pass
                info.pass_configs.push_back({});
                info.pass_verts.push_back({});
                info.pass_frags.push_back({});

                info.pass_configs.at(info.pass_configs.size() - 1).append(std::move(arr[i]));
            }
            if (step == kVert)
            {
                info.pass_verts.at(info.pass_configs.size() - 1).append(std::move(arr[i]));
            }
            if (step == kFrag)
            {
                info.pass_frags.at(info.pass_configs.size() - 1).append(std::move(arr[i]));
            }
        }
    }


    Shader::Shader()
    {
        this->id_ = 0;
    }

    Shader::~Shader()
    {
        if (this->GetIsBindGPU())
        {
            Logger::Log(std::format("unbind in destructor: {}", this->name_), LogLevel::Warning);
            this->UnBindGPU();
        }
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
