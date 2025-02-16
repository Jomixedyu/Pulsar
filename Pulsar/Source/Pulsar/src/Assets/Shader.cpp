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

        self->SetReady(true);

        return self.get();
    }


    void Shader::OnInstantiateAsset(AssetObject* obj)
    {
        base::OnInstantiateAsset(obj);
        Shader* self = static_cast<Shader*>(obj);

        self->m_shaderSource = m_shaderSource;
        self->m_featureOptions = m_featureOptions;
        *self->m_passName = *m_passName;
        *self->m_preDefines = *m_preDefines;
        self->Initialize();
    }

    Shader::Shader()
    {
        init_sptr_member(m_passName);
        init_sptr_member(m_preDefines);
    }

    void Shader::Serialize(AssetSerializer* s)
    {
        base::Serialize(s);

        if (s->ExistStream)
        {
            ReadWriteStream(s->Stream, s->IsWrite, m_shaderSource);
        }

        if (!s->IsWrite) // read
        {
            m_preDefines->clear();
            m_featureOptions.clear();

            auto passes = s->Object->At("Passes");
            *m_passName = passes->AsString();

            Initialize();
        }
        else
        {
            auto passNameString = s->Object->New(ser::VarientType::String);
            passNameString->Assign(*m_passName);

            s->Object->Add("Passes", passNameString);

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
        return std::ranges::any_of(m_shaderSource.ApiMaps, [=](auto& p){ return p.first == api; });
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
        case ShaderParameterType::Texture: {
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

    void Shader::SetReady(bool b)
    {
        if (m_isReady == b) return;
        m_isReady = b;
        if (b)
        {
            RuntimeObjectManager::NotifyDependObjects(GetObjectHandle(), DependencyObjectState::Reload);
        }
        else
        {
            RuntimeObjectManager::NotifyDependObjects(GetObjectHandle(), DependencyObjectState::Unload);
        }
    }
    void Shader::Initialize()
    {
        const auto currentApi = Application::GetGfxApp()->GetApiType();
        if (!m_shaderSource.ApiMaps.contains(currentApi))
        {
            SetReady(false);
            return;
        }

        auto configJson = m_shaderSource.ApiMaps.at(currentApi).Config;
        if (configJson.empty())
        {
            SetReady(false);
            return;
        }
        try
        {
            m_shaderConfig = ser::JsonSerializer::Deserialize<ShaderPassConfig>(configJson);
        }
        catch(const std::exception&)
        {
            SetReady(false);
            return;
        }

        SetReady(true);

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
