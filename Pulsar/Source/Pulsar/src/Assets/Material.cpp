#include "Assets/Material.h"
#include "Application.h"
#include "Assets/StaticMesh.h"
#include "Logger.h"

#include <CoreLib.Serialization/JsonSerializer.h>
#include <Pulsar/AssetManager.h>
#include <Pulsar/Assets/Material.h>

namespace pulsar
{
    static auto GetVertexLayout(gfx::GFXApplication* app)
    {
        auto vertDescLayout = app->CreateVertexLayoutDescription();
        vertDescLayout->BindingPoint = 0;
        vertDescLayout->Stride = sizeof(StaticMeshVertex);

        vertDescLayout->Attributes.push_back({(int)EngineInputSemantic::POSITION, gfx::GFXVertexInputDataFormat::R32G32B32_SFloat, offsetof(StaticMeshVertex, Position)});
        vertDescLayout->Attributes.push_back({(int)EngineInputSemantic::NORMAL, gfx::GFXVertexInputDataFormat::R32G32B32_SFloat, offsetof(StaticMeshVertex, Normal)});
        vertDescLayout->Attributes.push_back({(int)EngineInputSemantic::TANGENT, gfx::GFXVertexInputDataFormat::R32G32B32_SFloat, offsetof(StaticMeshVertex, Tangent)});
        vertDescLayout->Attributes.push_back({(int)EngineInputSemantic::BITANGENT, gfx::GFXVertexInputDataFormat::R32G32B32_SFloat, offsetof(StaticMeshVertex, Bitangent)});
        vertDescLayout->Attributes.push_back({(int)EngineInputSemantic::COLOR, gfx::GFXVertexInputDataFormat::R32G32B32_SFloat, offsetof(StaticMeshVertex, Color)});

        for (size_t i = 0; i < STATICMESH_MAX_TEXTURE_COORDS; i++)
        {
            vertDescLayout->Attributes.push_back({(int)EngineInputSemantic::TEXCOORD0 + i, gfx::GFXVertexInputDataFormat::R32G32_SFloat, offsetof(StaticMeshVertex, TexCoords[i])});
        }

        return vertDescLayout;
    }

    ObjectPtr<Material> Material::StaticCreate(string_view name, Shader_ref shader)
    {
        Material_sp material = mksptr(new Material);
        material->Construct();
        material->SetName(name);
        material->m_shader = shader;

        return material;
    }

    void Material::OnConstruct()
    {
    }

    bool Material::CreateGPUResource()
    {
        if (m_createdGpuResource)
        {
            return true;
        }
        if (!m_shader || m_shader->GetSourceData().ApiMaps.empty())
        {
            return false;
        }
        m_createdGpuResource = true;

        const auto& passes = m_shader->GetSourceData().ApiMaps.at(Application::GetGfxApp()->GetApiType());

        ShaderPassConfig_sp shaderpassConfig;

        // process deferred
        // create shader module from source
        gfx::GFXGpuProgram_sp gpuProgram = Application::GetGfxApp()->CreateGpuProgram(passes.Sources);

        // create shader pass state config
        gfx::GFXShaderPassConfig config{};
        {
            ShaderPassConfig_sp sourceConfig;
            try
            {
                sourceConfig = ser::JsonSerializer::Deserialize<ShaderPassConfig>(passes.Config);
            }
            catch (const std::exception& e)
            {
                Logger::Log("shader config json error!", LogLevel::Error);
                m_createdGpuResource = false;
                return false;
            }

            config.CullMode = sourceConfig->CullMode;
            config.DepthCompareOp = sourceConfig->DepthCompareOp;
            config.DepthTestEnable = sourceConfig->DepthTestEnable;
            config.DepthWriteEnable = sourceConfig->DepthWriteEnable;
            config.StencilTestEnable = sourceConfig->StencilTestEnable;
            shaderpassConfig = sourceConfig;
        }


        // create shader pass
        m_gfxShaderPasses = Application::GetGfxApp()->CreateShaderPass(
            config,
            gpuProgram,
            {GetVertexLayout(Application::GetGfxApp())});


        // create constant layouts and constant buffer
        array_list<gfx::GFXDescriptorSetLayoutInfo> descLayoutInfos;

        // create constant buffer
        size_t constantBufferSize{};
        if (shaderpassConfig->ConstantProperties)
        {
            size_t offset = 0;
            for (const auto element : *shaderpassConfig->ConstantProperties)
            {
                m_propertyInfo.insert({ index_string{element->Name}, { offset, element->Type } });
                switch (element->Type)
                {
                case ShaderParameterType::Scalar:
                    offset += sizeof(float);
                    break;
                case ShaderParameterType::Vector:
                    offset += sizeof(Vector4f);
                    break;
                default:;
                }
                constantBufferSize = offset;
            }
        }
        if (constantBufferSize)
        {
            m_materialConstantBuffer =
                Application::GetGfxApp()->CreateBuffer(gfx::GFXBufferUsage::ConstantBuffer, constantBufferSize);
        }


        if (constantBufferSize != 0)
        {
            auto info = gfx::GFXDescriptorSetLayoutInfo {
                gfx::GFXDescriptorType::ConstantBuffer,
                gfx::GFXShaderStageFlags::VertexFragment,
                0, 3
            };
            descLayoutInfos.push_back(info);
        }


        // create texture parameter layout
        if (shaderpassConfig->Properties)
        {
            auto offset = constantBufferSize != 0 ? 1 : 0;
            const auto count = shaderpassConfig->Properties->size();
            for (size_t i = 0; i < count; ++i)
            {
                //auto& item = shaderpassConfig->Properties->at(i);
                auto info = gfx::GFXDescriptorSetLayoutInfo {
                    gfx::GFXDescriptorType::ConstantBuffer,
                    gfx::GFXShaderStageFlags::VertexFragment,
                    (uint32_t)i+offset, 3
                };
                descLayoutInfos.push_back(info);
            }
        }

        // create descriptorset layouts
        m_descriptorSetLayout = Application::GetGfxApp()->CreateDescriptorSetLayout(
            descLayoutInfos.data(),
            descLayoutInfos.size());


        // create descriptor set
        m_descriptorSet = Application::GetGfxApp()->GetDescriptorManager()->GetDescriptorSet(m_descriptorSetLayout);
        if (m_materialConstantBuffer)
        {
            m_descriptorSet->AddDescriptor("PerMat", 0)->SetConstantBuffer(m_materialConstantBuffer.get());
        }
        m_descriptorSet->Submit();

        return true;
    }

    void Material::DestroyGPUResource()
    {
        if (!m_createdGpuResource)
        {
            return;
        }
        m_createdGpuResource = false;
        m_gfxShaderPasses.reset();
        m_descriptorSet.reset();
    }
    bool Material::IsCreatedGPUResource() const
    {
        return m_createdGpuResource;
    }

    class MaterialSerializationData : public Object
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::MaterialSerializationData, Object);

    public:
        CORELIB_REFL_DECL_FIELD(Shader);
        Shader_ref Shader;
    };
    CORELIB_DECL_SHORTSPTR(MaterialSerializationData);

    void Material::Serialize(AssetSerializer* s)
    {
        // MaterialSerializationData_sp data;
        if (s->IsWrite)
        {
            auto shaderObjectHandle = s->Object->New(ser::VarientType::String);
            shaderObjectHandle->Assign(m_shader.handle.to_string());
            s->Object->Add("Shader", shaderObjectHandle);
        }
        else
        {
            // string json;
            // ReadWriteTextStream(s->Stream, s->IsWrite, json);
            // auto data = ser::JsonSerializer::Deserialize<MaterialSerializationData>(json);
            // m_shader = data->Shader;
            // InitObjectPtr(m_shader);
            auto id = ObjectHandle::parse(s->Object->At("Shader")->AsString());
            m_shader = id;
            TryFindOrLoadObject(m_shader);
        }
    }

    void Material::SetFloat(const index_string& name, float value)
    {
        m_isDirtyParameter = true;
        m_parameterValues[name].SetValue(value);
        // m_descriptorSet->Find("ShaderParameter")->IsDirty = true;
    }

    void Material::SetTexture(const index_string& name, Texture_ref value)
    {
        m_isDirtyParameter = true;
        m_parameterValues[name].SetValue(value);
    }

    void Material::SetVector4(const index_string& name, const Vector4f& value)
    {
        m_isDirtyParameter = true;
        m_parameterValues[name].SetValue(value);
    }

    float Material::GetFloat(const index_string& name)
    {
        auto it = m_parameterValues.find(name);
        if (it == m_parameterValues.end())
        {
            return 0;
        }
        return it->second.AsFloat();
    }

    Vector4f Material::GetVector4(const index_string& name)
    {
        auto it = m_parameterValues.find(name);
        if (it == m_parameterValues.end())
        {
            return Vector4f{};
        }
        return it->second.AsVector();
    }
    Texture_ref Material::GetTexture(const index_string& name)
    {
        auto it = m_parameterValues.find(name);
        if (it == m_parameterValues.end())
        {
            return Texture_ref{};
        }
        return it->second.AsTexture();
    }


    void Material::SubmitParameters()
    {
        if (!m_isDirtyParameter)
        {
            return;
        }

        // copy parameter

        m_bufferData.resize(m_materialConstantBuffer->GetSize());
        for (auto value : m_parameterValues)
        {
            auto ptr = m_bufferData.data() + value.second.Offset;

            switch (value.second.Type)
            {
            case MaterialParameterValue::FLOAT: {
                *(float*)ptr = value.second.AsFloat();
                break;
            }
            case MaterialParameterValue::VECTOR: {
                *(Vector4f*)ptr = value.second.AsVector();
                break;
            }
            case MaterialParameterValue::TEXTURE:
                break;
            default:;
            }
        }

        m_materialConstantBuffer->Fill(m_bufferData.data());


        m_descriptorSet->Find("PerMat")->SetConstantBuffer(m_materialConstantBuffer.get());

        m_isDirtyParameter = false;
        m_descriptorSet->Submit();
    }

    void Material::PostEditChange(FieldInfo* info)
    {
        base::PostEditChange(info);
        auto& name = info->GetName();
        if (name == NAMEOF(m_shader))
        {
            SetShader(m_shader);
        }
    }
    Shader_ref Material::GetShader() const
    {
        if (m_shader)
        {
            return m_shader;
        }
        return Application::inst()->GetAssetManager()->LoadAsset<Shader>("Engine/Shaders/Missing");
    }
    void Material::SetShader(Shader_ref value)
    {
        m_shader = value;
        OnShaderChanged.Invoke();
    }

    gfx::GFXShaderPass_sp Material::GetGfxShaderPass()
    {
        if (!m_gfxShaderPasses)
        {
            auto missing = GetAssetManager()->LoadAsset<Material>("Engine/Materials/Missing");
            missing->CreateGPUResource();
            return missing->GetGfxShaderPass();
        }
        return m_gfxShaderPasses;
    }

} // namespace pulsar