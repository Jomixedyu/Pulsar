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
        self->m_name = name;
        self->m_shaderSource = std::move(pass);

        self->CreateGPUResource();

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
        m_passNames = mksptr(new List<String_sp>);
        m_preDefines = mksptr(new List<String_sp>);
    }
    void Shader::Serialize(AssetSerializer* s)
    {
        base::Serialize(s);
        if (!s->IsWrite) // read
        {
            m_passNames->clear();
            m_preDefines->clear();

            auto passes = s->Object->At("Passes");
            for (size_t i = 0; i < passes->GetCount(); i++)
            {
                m_passNames->push_back(mkbox(passes->At(i)->AsString()));
            }
        }
        else
        {
            auto passNameArray = s->Object->New(ser::VarientType::Array);
            for (auto& passName : *m_passNames)
            {
                passNameArray->Push(*passName);
            }
            s->Object->Add("Passes", passNameArray);
        }
        if (s->ExistStream)
        {
            ReadWriteStream(s->Stream, s->IsWrite, m_shaderSource);
        }
    }

    static auto _GetVertexLayout(gfx::GFXApplication* app)
    {
        auto vertDescLayout = app->CreateVertexLayoutDescription();
        vertDescLayout->BindingPoint = 0;
        vertDescLayout->Stride = sizeof(StaticMeshVertex);

        vertDescLayout->Attributes.push_back({ (int)EngineInputSemantic::POSITION, gfx::GFXVertexInputDataFormat::R32G32B32_SFloat, offsetof(StaticMeshVertex, Position) });
        vertDescLayout->Attributes.push_back({ (int)EngineInputSemantic::NORMAL, gfx::GFXVertexInputDataFormat::R32G32B32_SFloat, offsetof(StaticMeshVertex, Normal) });
        vertDescLayout->Attributes.push_back({ (int)EngineInputSemantic::TANGENT, gfx::GFXVertexInputDataFormat::R32G32B32_SFloat, offsetof(StaticMeshVertex, Tangent) });
        vertDescLayout->Attributes.push_back({ (int)EngineInputSemantic::BITANGENT, gfx::GFXVertexInputDataFormat::R32G32B32_SFloat, offsetof(StaticMeshVertex, Bitangent) });
        vertDescLayout->Attributes.push_back({ (int)EngineInputSemantic::COLOR, gfx::GFXVertexInputDataFormat::R32G32B32_SFloat, offsetof(StaticMeshVertex, Color) });

        for (size_t i = 0; i < STATICMESH_MAX_TEXTURE_COORDS; i++)
        {
            vertDescLayout->Attributes.push_back({ (int)EngineInputSemantic::TEXCOORD0 + i, gfx::GFXVertexInputDataFormat::R32G32_SFloat, offsetof(StaticMeshVertex, TexCoords[i]) });
        }

        return vertDescLayout;
    }

    void Shader::CreateGPUResource()
    {
        auto& passes = m_shaderSource.ApiMaps[Application::GetGfxApp()->GetApiType()].Passes;

        for (size_t i = 0; i < passes.size(); i++)
        {
            // create shader module from source
            gfx::GFXGpuProgram_sp gpuProgram = Application::GetGfxApp()->CreateGpuProgram(passes[i].Sources);

            // create shader pass state config
            gfx::GFXShaderPassConfig config{};
            {
                auto sourceConfig = ser::JsonSerializer::Deserialize<ShaderPassConfig>(passes[i].Config);
                config.CullMode = sourceConfig->CullMode;
                config.DepthCompareOp = sourceConfig->DepthCompareOp;
                config.DepthTestEnable = sourceConfig->DepthTestEnable;
                config.DepthWriteEnable = sourceConfig->DepthWriteEnable;
                config.StencilTestEnable = sourceConfig->StencilTestEnable;
                config.Topology = (gfx::GFXPrimitiveTopology)sourceConfig->Topology;
            }

            // create descriptor layout
            std::shared_ptr<gfx::GFXDescriptorSetLayout> descriptorSetLayout;
            {
                array_list<gfx::GFXDescriptorSetLayoutInfo> descriptorLayoutInfos;

                gfx::GFXDescriptorSetLayoutInfo commonInfo(
                    0,
                    gfx::GFXDescriptorType::ConstantBuffer,
                    gfx::GFXShaderStageFlags::VertexFragment);

                descriptorLayoutInfos.push_back(commonInfo);
                descriptorSetLayout = Application::GetGfxApp()->CreateDescriptorSetLayout(descriptorLayoutInfos);
            }

            // create shader pass
            auto shaderPass = Application::GetGfxApp()->CreateShaderPass(
                config,
                gpuProgram,
                descriptorSetLayout,
                { _GetVertexLayout(Application::GetGfxApp()) });

            m_gfxShaderPass.push_back(shaderPass);
        }
    }

    void Shader::DestroyGPUResource()
    {
        m_gfxShaderPass.clear();
    }

    bool Shader::IsCreatedGPUResource() const
    {
        return false;
    }
    void Shader::OnDestroy()
    {
        base::OnDestroy();
        if (IsCreatedGPUResource())
        {
            DestroyGPUResource();
        }
    }

    void Shader::ResetShaderSource(const ShaderSourceData& serData)
    {
        m_shaderSource = serData;
        auto created = IsCreatedGPUResource();
        if (created)
        {
            DestroyGPUResource();
        }
        if (created)
        {
            CreateGPUResource();
        }
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

    static std::iostream& ReadWriteStream(std::iostream& stream, bool write, ShaderSourceData::Pass& data)
    {
        sser::ReadWriteStream(stream, write, data.Config);
        sser::ReadWriteStream(stream, write, data.Sources);
        return stream;
    }
    static std::iostream& ReadWriteStream(std::iostream& stream, bool write, ShaderSourceData::ApiPlatform& data)
    {
        sser::ReadWriteStream(stream, write, data.Passes);
        return stream;
    }
    std::iostream& ReadWriteStream(std::iostream& stream, bool write, ShaderSourceData& data)
    {
        using namespace sser;
        using namespace ser;

        sser::ReadWriteStream(stream, write, data.ApiMaps);

        //sser::ReadWriteStream(stream, write, data.Config);
        //sser::ReadWriteStream(stream, write, data.Sources);

        return stream;
    }

}
