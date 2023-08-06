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

    void Shader::OnSerialize(AssetSerializer* serializer)
    {

    }

    sptr<Shader> Shader::StaticCreate(string_view name, array_list<ShaderPassSerializeData>&& pass)
    {
        Shader_sp self = mksptr(new Shader);
        self->name_ = name;
        self->Construct();
        self->m_shaderSource = std::move(pass);

        self->BindGPU();

        return self;
    }

    static auto _GetVertexLayout(gfx::GFXApplication* app)
    {
        auto vertDescLayout = app->CreateVertexLayoutDescription();
        vertDescLayout->BindingPoint = 0;
        vertDescLayout->Stride = sizeof(StaticMeshVertex);

        vertDescLayout->Attributes.push_back({ gfx::GFXVertexInputDataFormat::R32G32B32_SFloat, offsetof(StaticMeshVertex, Position) });
        vertDescLayout->Attributes.push_back({ gfx::GFXVertexInputDataFormat::R32G32B32_SFloat, offsetof(StaticMeshVertex, Normal) });
        vertDescLayout->Attributes.push_back({ gfx::GFXVertexInputDataFormat::R32G32B32_SFloat, offsetof(StaticMeshVertex, Tangent) });
        vertDescLayout->Attributes.push_back({ gfx::GFXVertexInputDataFormat::R32G32B32_SFloat, offsetof(StaticMeshVertex, Bitangent) });
        vertDescLayout->Attributes.push_back({ gfx::GFXVertexInputDataFormat::R32G32B32_SFloat, offsetof(StaticMeshVertex, Color) });

        for (size_t i = 0; i < STATICMESH_MAX_TEXTURE_COORDS; i++)
        {
            vertDescLayout->Attributes.push_back({ gfx::GFXVertexInputDataFormat::R32G32_SFloat, offsetof(StaticMeshVertex, TexCoords[i]) });
        }

        return vertDescLayout;
    }

    void Shader::BindGPU()
    {

        for (size_t i = 0; i < m_shaderSource.size(); i++)
        {
            // create shader module from source
            auto gpuProgram = Application::GetGfxApp()->CreateGpuProgram(m_shaderSource[i].VertBytes, m_shaderSource[i].PixelBytes);

            // create shader pass state config
            gfx::GFXShaderPassConfig config{};
            {
                auto& sourceConfig = m_shaderSource[i].Config;
                config.CullMode = sourceConfig->CullMode;
                config.DepthCompareOp = sourceConfig->DepthCompareOp;
                config.DepthTestEnable = sourceConfig->DepthTestEnable;
                config.DepthWriteEnable = sourceConfig->DepthWriteEnable;
                config.StencilTestEnable = sourceConfig->StencilTestEnable;
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
                _GetVertexLayout(Application::GetGfxApp()));

            m_shaderPass.push_back(shaderPass);
        }
    }

    void Shader::UnBindGPU()
    {
    }

    bool Shader::GetIsBindGPU()
    {
        return false;
    }

    ser::Stream& ReadWriteStream(ser::Stream& stream, bool isWrite, ShaderPassSerializeData& data)
    {
        using namespace ser;
        if (isWrite)
        {
            auto json = ser::JsonSerializer::Serialize(data.Config.get(), {});
            ReadWriteStream(stream, isWrite, json);
        }
        else
        {
            string json;
            ReadWriteStream(stream, isWrite, json);
            data.Config = ser::JsonSerializer::Deserialize<ShaderPassConfig>(json);
        }

        ReadWriteStream(stream, isWrite, data.VertBytes);
        ReadWriteStream(stream, isWrite, data.PixelBytes);
        return stream;
    }

}
