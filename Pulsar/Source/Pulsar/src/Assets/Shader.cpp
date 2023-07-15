#include "Assets/Shader.h"

#include <Pulsar/Assets/Shader.h>

#include <Pulsar/Assets/Texture.h>
#include <CoreLib.Serialization/JsonSerializer.h>

#include <Pulsar/Logger.h>

namespace pulsar
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

        enum
        {
            kShaderConfig = 1,
            kNewPass = 1 << 1,
            kPass = 1 << 2,
            kVert = 1 << 3,
            kFrag = 1 << 4
        };

        for (size_t i = 0; i < arr.size(); i++)
        {
            if (arr[i].starts_with("#pragma config"))
            {
                auto configs = StringUtil::Split(arr[i], u8char(" "));
                info.shader_name = configs[configs.size() - 1];
                step = kShaderConfig;
                continue;
            }
            if (arr[i].starts_with("#pragma pass"))
            {
                step = kNewPass | kPass;
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

            if (step & kPass)
            {
                // new pass
                if (step & kNewPass)
                {
                    info.pass_configs.emplace_back();
                    info.pass_verts.emplace_back();
                    info.pass_frags.emplace_back();
                    step &= ~kNewPass;
                }

                info.pass_configs.at(info.pass_configs.size() - 1).append(std::move(arr[i]));
                info.pass_configs.at(info.pass_configs.size() - 1).append("\n");
            }
            if (step == kVert)
            {
                info.pass_verts.at(info.pass_verts.size() - 1).append(std::move(arr[i]));
                info.pass_verts.at(info.pass_verts.size() - 1).append("\n");
            }
            if (step == kFrag)
            {
                info.pass_frags.at(info.pass_frags.size() - 1).append(std::move(arr[i]));
                info.pass_frags.at(info.pass_frags.size() - 1).append("\n");
            }
        }

        return info;
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

    void Shader::UseShader(int32_t pass_index)
    {
        
    }

    ShaderPass* Shader::GetPass(int32_t index)
    {
        assert(index >= 0 && index < this->pass_.size());
        return this->pass_[index];
    }

    static bool _CheckShaderProgram(int id)
    {
        int isSuccess;

        return isSuccess;
    }
    static bool _CheckShaderCompile(const uint32_t& shaderId)
    {
        int isSuccess;

        return isSuccess;
    }
    string _GetShaderCompileErrorInfo(const uint32_t& shaderId)
    {
        char info[512];

        return string(info);
    }

    int32_t Shader::GetUniformLocaltion(std::string_view name)
    {
        return 0;
    }

    void Shader::SetUniformInt(std::string_view name, const int32_t& i)
    {

    }

    void Shader::SetUniformFloat(std::string_view name, const float& f)
    {

    }

    void Shader::SetUniformMatrix4fv(std::string_view name, const float* value)
    {

    }

    void Shader::SetUniformMatrix4fv(std::string_view name, const Matrix4f& mat)
    {
        SetUniformMatrix4fv(name, mat.get_value_ptr());
    }

    void Shader::SetUniformVector3(std::string_view name, const Vector3f& value)
    {

    }

    void Shader::SetUniformColor(std::string_view name, const LinearColorf& value)
    {

    }

    void Shader::SetUniformColor(std::string_view name, const Vector3f& value)
    {
        SetUniformColor(name, LinearColorf(value.x, value.y, value.z));
    }

    void Shader::SetUniformTexture(std::string_view name, Texture_rsp tex)
    {
        assert(tex->GetIsBindGPU());
        //this->SetUniformInt(name, tex->get_tex_id());
    }

    void Shader::BindGPU()
    {
        assert(!this->GetIsBindGPU());

    }

    void Shader::UnBindGPU()
    {
        if (this->GetIsBindGPU())
        {
            this->id_ = 0;
        }
    }

    bool Shader::GetIsBindGPU()
    {
        return this->id_ != 0;
    }


    Shader_sp Shader::StaticCreate(string_view name, array_list<ShaderPass*>&& pass)
    {
        auto shader = mksptr(new Shader);
        shader->Construct();
        shader->set_name(name);
        shader->pass_ = std::move(pass);
        return shader;
    }
    sptr<Shader> Shader::StaticCreate(const string& shader_source)
    {
        auto info = _ParseShaderInfo(shader_source);

        auto shader = mksptr(new Shader);
        shader->Construct();
        shader->set_name(info.shader_name);

        for (size_t i = 0; i < info.pass_configs.size(); i++)
        {
            shader->pass_.push_back(new ShaderPass(info.shader_name.c_str(), info.pass_verts[i].c_str(), info.pass_frags[i].c_str()));
        }
        return shader;
    }
}
