#include "Assets/Shader.h"

#include <Pulsar/Assets/Shader.h>

#include <Pulsar/Assets/Texture.h>
#include <CoreLib.Serialization/JsonSerializer.h>

#include <Pulsar/Logger.h>
#include "Application.h"
#include "Assets/StaticMesh.h"

namespace pulsar
{
    using namespace std;

    sptr<Shader> Shader::StaticCreate(string_view name, ShaderSourceData&& pass)
    {
        Shader_sp self = mksptr(new Shader);
        self->Construct();
        self->SetName(name);
        self->m_shaderSource = std::move(pass);

        return self;
    }

    static size_t BeginBinaryField(std::iostream& stream, bool write, string& name)
    {
        auto pos = stream.tellp();
        sser::ReadWriteStream(stream, write, name);
        return pos;
    }
    static void EndBinaryField(std::iostream& stream, bool write, std::streampos start)
    {
        auto pos = stream.tellp();
        stream.seekp(start);
        size_t size = pos - start;
        sser::ReadWriteStream(stream, write, size);
    }
    Shader::Shader()
    {
        init_sptr_member(m_passName);
        init_sptr_member(m_preDefines);
        init_sptr_member(m_shaderConfig);
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

            m_renderingType = (ShaderPassRenderingType)s->Object->At("RenderingType")->AsInt();

            if (s->HasEditorData)
            {
                if (s->Object->ContainsKey("Available"))
                {
                    m_isAvailable = s->Object->At("Available")->AsBool();
                }
            }
        }
        else
        {
            auto passNameString = s->Object->New(ser::VarientType::String);
            passNameString->Assign(*m_passName);

            s->Object->Add("Passes", passNameString);

            auto config = s->Object->New(ser::VarientType::Object);
            config->AssignParse(ser::JsonSerializer::Serialize(m_shaderConfig.get(), {}));
            s->Object->Add("Config", config);

            s->Object->Add("RenderingType", (int)m_renderingType);

            if (s->HasEditorData)
            {
                auto available = s->Object->New(ser::VarientType::Bool);
                available->Assign(m_isAvailable);
                s->Object->Add("Available", available);
            }
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

    void Shader::ResetShaderSource(const ShaderSourceData& serData)
    {
        m_shaderSource = serData;
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
