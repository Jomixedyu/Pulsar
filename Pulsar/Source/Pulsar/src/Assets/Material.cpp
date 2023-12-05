#include "Assets/Material.h"
#include "Application.h"
#include "Assets/StaticMesh.h"

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

    void Material::SetInt(const index_string& name, int value)
    {
    }

    void Material::CreateGPUResource()
    {
        if (m_createdGpuResource)
        {
            return;
        }
        m_createdGpuResource = true;

        const auto& passes = m_shader->GetSourceData().ApiMaps.at(Application::GetGfxApp()->GetApiType()).Passes;

        for (size_t i = 0; i < passes.size(); i++)
        {
            // create shader module from source
            gfx::GFXGpuProgram_sp gpuProgram = Application::GetGfxApp()->CreateGpuProgram(passes[i].Sources);

            // create shader pass state config
            gfx::GFXShaderPassConfig config{};
            {
                const auto sourceConfig = ser::JsonSerializer::Deserialize<ShaderPassConfig>(passes[i].Config);
                config.CullMode = sourceConfig->CullMode;
                config.DepthCompareOp = sourceConfig->DepthCompareOp;
                config.DepthTestEnable = sourceConfig->DepthTestEnable;
                config.DepthWriteEnable = sourceConfig->DepthWriteEnable;
                config.StencilTestEnable = sourceConfig->StencilTestEnable;
                config.Topology = (gfx::GFXPrimitiveTopology)sourceConfig->Topology;
            }

            // create descriptor layout
            {
                gfx::GFXDescriptorSetLayoutInfo descriptorLayoutInfo {
                    gfx::GFXDescriptorType::ConstantBuffer,
                    gfx::GFXShaderStageFlags::VertexFragment,
                    0, 2
                };
                m_descriptorSetLayout = Application::GetGfxApp()->CreateDescriptorSetLayout(&descriptorLayoutInfo, 1);
            }

            {


                // create shader pass
                auto shaderPass = Application::GetGfxApp()->CreateShaderPass(
                    config,
                    gpuProgram,
                    {GetVertexLayout(Application::GetGfxApp())});

                m_gfxShaderPasses.push_back(shaderPass);
            }

        }

        // m_descriptorSet = Application::GetGfxApp()->GetDescriptorManager()->GetDescriptorSet(m_descriptorSetLayout.get());
        // m_descriptorSet->AddDescriptor("ShaderParameter", 2)->SetConstantBuffer(m_materialBuffer.get());
        // m_descriptorSet->Submit();
    }
    void Material::DestroyGPUResource()
    {
        if (!m_createdGpuResource)
        {
            return;
        }
        m_createdGpuResource = false;
        m_gfxShaderPasses.clear();
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

    template <typename T>
    static void InitObjectPtr(ObjectPtr<T>& obj)
    {
        auto ptr = RuntimeObjectWrapper::GetObject(obj.handle);
        if (ptr)
        {
            obj.Ptr = (T*)ptr;
        }
        else
        {
            auto asset = GetAssetManager()->LoadAssetById(obj.handle);
            obj.Ptr = (T*)asset.Ptr;
        }
    }

    void Material::Serialize(AssetSerializer* s)
    {
        // MaterialSerializationData_sp data;
        if (s->IsWrite)
        {
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
            InitObjectPtr(m_shader);
        }
    }

    void Material::SetFloat(const index_string& name, float value)
    {
        m_isDirtyParameter = true;
        m_parameterValues[name].SetValue(value);
        // m_descriptorSet->Find("ShaderParameter")->IsDirty = true;
    }

    void Material::SetColor(const index_string& name, const Color4f& value)
    {
        m_isDirtyParameter = true;
        m_parameterValues[name].SetValue(value);
    }

    void Material::SetTexture(const index_string& name, Texture_ref value)
    {
        m_isDirtyParameter = true;
        m_parameterValues[name].SetValue(value);
    }
    void Material::SetMatrix4f(const index_string& name, const Matrix4f& value)
    {
        m_isDirtyParameter = true;
        m_parameterValues[name].SetValue(value);
    }
    void Material::SetVector4(const index_string& name, const Vector4f& value)
    {
        m_isDirtyParameter = true;
        m_parameterValues[name].SetValue(value);
    }
    int Material::GetInt(const index_string& name)
    {
        auto it = m_parameterValues.find(name);
        if (it == m_parameterValues.end())
        {
            return 0;
        }
        return it->second.AsInt();
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
    Color4f Material::GetColor(const index_string& name)
    {
        auto it = m_parameterValues.find(name);
        if (it == m_parameterValues.end())
        {
            return Color4f{};
        }
        return it->second.AsColor();
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
    Matrix4f Material::GetMatrix4f(const index_string& name)
    {
        auto it = m_parameterValues.find(name);
        if (it == m_parameterValues.end())
        {
            return Matrix4f{};
        }
        return it->second.AsMatrix();
    }

    void Material::CommitParameters()
    {
        if (!m_isDirtyParameter)
        {
            return;
        }
        // copy parameter

        m_descriptorSet->Find("ShaderParameter")->SetConstantBuffer(m_materialBuffer.get());

        m_isDirtyParameter = false;
        m_descriptorSet->Submit();
    }

    Shader_ref Material::GetShader() const
    {
        if (m_shader)
        {
            return m_shader;
        }
        return Application::inst()->GetAssetManager()->LoadAsset<Shader>("Engine/Shaders/Missing");
    }
} // namespace pulsar