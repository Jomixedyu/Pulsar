#include "Assets/Shader.h"

#include <Pulsar/Assets/Shader.h>

#include <Pulsar/Assets/Texture.h>
#include <CoreLib.Serialization/JsonSerializer.h>

#include "Application.h"
#include "AssetManager.h"
#include "Assets/StaticMesh.h"
#include "BuiltinAsset.h"
#include <Pulsar/Logger.h>

#include <ranges>

namespace pulsar
{
    using namespace std;

    RCPtr<Shader> Shader::StaticCreate(string_view name, ShaderSourceData&& pass)
    {
        Shader_sp self = mksptr(new Shader);
        self->Construct();
        self->SetName(name);
        self->m_shaderSource = std::move(pass);

        return self.get();
    }

    // static size_t BeginBinaryField(std::iostream& stream, bool write, string& name)
    // {
    //     auto pos = stream.tellp();
    //     sser::ReadWriteStream(stream, write, name);
    //     return pos;
    // }
    // static void EndBinaryField(std::iostream& stream, bool write, std::streampos start)
    // {
    //     auto pos = stream.tellp();
    //     stream.seekp(start);
    //     size_t size = pos - start;
    //     sser::ReadWriteStream(stream, write, size);
    // }
    Shader::Shader()
    {
        init_sptr_member(m_passName);
        init_sptr_member(m_preDefines);
    }

    void Shader::Serialize(AssetSerializer* s)
    {
        base::Serialize(s);
        if (!s->IsWrite) // read
        {
            m_preDefines->clear();

            auto passes = s->Object->At("Passes");
            *m_passName = passes->AsString();

            m_shaderConfig = ser::JsonSerializer::Deserialize<ShaderPassConfig>(s->Object->At("Config")->ToString());
            Initialize();
        }
        else
        {
            auto passNameString = s->Object->New(ser::VarientType::String);
            passNameString->Assign(*m_passName);

            s->Object->Add("Passes", passNameString);

            auto config = s->Object->New(ser::VarientType::Object);
            config->AssignParse(ser::JsonSerializer::Serialize(m_shaderConfig.get(), {}));
            s->Object->Add("Config", config);


        }

        if (s->ExistStream)
        {
            ReadWriteStream(s->Stream, s->IsWrite, m_shaderSource);
        }
    }

    void Shader::OnDestroy()
    {
        base::OnDestroy();

    }

    void Shader::ResetShaderSource(ShaderSourceData&& serData)
    {
        m_shaderSource = std::move(serData);
        auto& cfg = m_shaderSource.ApiMaps.at(Application::GetGfxApp()->GetApiType()).Config;
        m_shaderConfig = ser::JsonSerializer::Deserialize<ShaderPassConfig>(cfg);

        Initialize();
        SendOuterDependencyMsg(DependencyObjectState::Reload);
    }

    ShaderPassConfig* Shader::GetConfig() const
    {
        return m_shaderConfig.get();
    }

    array_list<gfx::GFXApi> Shader::GetSupportedApi() const
    {
        array_list<gfx::GFXApi> ret;
        for (auto& [k, v] : m_shaderSource.ApiMaps)
        {
            ret.push_back(k);
        }
        return ret;
    }
    bool Shader::HasSupportedApiData(gfx::GFXApi api) const
    {
        for (auto& [k, v] : m_shaderSource.ApiMaps)
        {
            if (k == api)
                return true;
        }
        return false;
    }
    array_list<index_string> Shader::GetPropertyNames() const
    {
        return m_propertyInfo | std::views::keys | std::ranges::to<array_list<index_string>>();
    }
    const MaterialParameterInfo* Shader::GetPropertyInfo(index_string name) const
    {
        auto it = m_propertyInfo.find(name);
        if (it != m_propertyInfo.end())
            return &it->second;
        return nullptr;
    }

    static MaterialParameterValue ParseDefaultValue(const string& value, ShaderParameterType type)
    {
        MaterialParameterValue ret{};
        switch (type)
        {
        case ShaderParameterType::IntScalar:
            if (value.empty())
            {
                ret.SetValue(0);
            }
            else
            {
                ret.SetValue(std::atoi(value.c_str()));
            }
            break;
        case ShaderParameterType::Scalar:
            if (value.empty())
            {
                ret.SetValue(0.f);
            }
            else
            {
                ret.SetValue((float)std::atof(value.c_str()));
            }
            break;
        case ShaderParameterType::Vector: {
            if (value.empty())
            {
                ret.SetValue(Vector4f{});
            }
            else
            {
                auto strs = StringUtil::Split(value, ",");
                Vector4f v{
                    (float)std::atof(strs[0].c_str()),
                    (float)std::atof(strs[1].c_str()),
                    (float)std::atof(strs[2].c_str()),
                    (float)std::atof(strs[3].c_str())};
                ret.SetValue(v);
            }
            break;
        }
        case ShaderParameterType::Texture2D: {
            auto handle = ObjectHandle::parse(value);
            RCPtr<Texture2D> tex;
            if (handle.is_empty())
            {
                tex = GetAssetManager()->LoadAsset<Texture2D>(BuiltinAsset::Texture_White);
            }
            else
            {
                tex = handle;
            }

            ret.SetValue(tex);
            break;
        }
        }

        return ret;
    }

    void Shader::Initialize()
    {
        auto config = GetConfig();

        if (config->ConstantProperties)
        {
            size_t offset = 0;
            for (const auto& element : *config->ConstantProperties)
            {
                MaterialParameterInfo prop{};
                prop.Offset = offset;
                prop.Value = ParseDefaultValue(element->Value, element->Type);
                m_propertyInfo.insert({index_string{element->Name}, prop});

                switch (element->Type)
                {
                case ShaderParameterType::IntScalar:
                    offset += sizeof(int);
                    break;
                case ShaderParameterType::Scalar:
                    offset += sizeof(float);
                    break;
                case ShaderParameterType::Vector:
                    offset += sizeof(Vector4f);
                    break;
                default:;
                }
                m_constantBufferSize = offset;
            }
        }
        if (config->Properties)
        {
            const auto offset = config->ConstantProperties->empty() ? 0 : 1;
            const auto count = config->Properties->size();
            for (size_t i = 0; i < count; ++i)
            {
                const auto& item = config->Properties->at(i);
                MaterialParameterInfo prop{};
                prop.Offset = offset + i;
                prop.Value = ParseDefaultValue(item->Value, item->Type);

                m_propertyInfo.insert({index_string{item->Name}, prop});
            }
        }


    }

    static std::iostream& ReadWriteStream(std::iostream& stream, bool write, ShaderSourceData::ApiPlatform& data)
    {
        sser::ReadWriteStream(stream, write, data.Config);
        sser::ReadWriteStream(stream, write, data.Sources);
        return stream;
    }
    std::iostream& ReadWriteStream(std::iostream& stream, bool write, ShaderSourceData& data)
    {
        using namespace sser;
        using namespace ser;

        sser::ReadWriteStream(stream, write, data.ApiMaps);
        return stream;
    }

}
