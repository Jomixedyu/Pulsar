#include "Assets/Material.h"
#include <Pulsar/Assets/Material.h>
#include "Application.h"
#include <Pulsar/AssetManager.h>
#include <CoreLib.Serialization/JsonSerializer.h>

namespace pulsar
{

    ObjectPtr<Material> Material::StaticCreate(string_view name, Shader_ref shader)
    {
        Material_sp material = mksptr(new Material);
        material->Construct();
        material->SetName(name);
        material->m_shader = shader;

        return material;
    }
    //
    void Material::OnConstruct()
    {

    }

    void Material::CreateGPUResource()
    {
        m_createdGpuResource = true;
        if (!m_shader->IsCreatedGPUResource())
        {
            m_shader->CreateGPUResource();
        }
    }
    void Material::DestroyGPUResource()
    {
        m_createdGpuResource = false;

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



    template<typename T>
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
        //MaterialSerializationData_sp data;
        if (s->IsWrite)
        {

        }
        else
        {
            //string json;
            //ReadWriteTextStream(s->Stream, s->IsWrite, json);
            //auto data = ser::JsonSerializer::Deserialize<MaterialSerializationData>(json);
            //m_shader = data->Shader;
            //InitObjectPtr(m_shader);
            auto id = ObjectHandle::parse(s->Object->At("Shader")->AsString());
            m_shader = id;
            InitObjectPtr(m_shader);
        }

    }
    void Material::BeginGpu()
    {
        assert(HasObjectFlags(OF_Instance));
        //read config and add descriptorSet
        array_list<gfx::GFXDescriptorSetLayoutInfo> layoutInfos;
        // camera
        layoutInfos.push_back(gfx::GFXDescriptorSetLayoutInfo(0, gfx::GFXDescriptorType::ConstantBuffer, gfx::GFXShaderStageFlags::VertexFragment));
        // lighting
        layoutInfos.push_back(gfx::GFXDescriptorSetLayoutInfo(1, gfx::GFXDescriptorType::ConstantBuffer, gfx::GFXShaderStageFlags::VertexFragment));
        // shader pamaeters
        layoutInfos.push_back(gfx::GFXDescriptorSetLayoutInfo(2, gfx::GFXDescriptorType::ConstantBuffer, gfx::GFXShaderStageFlags::VertexFragment));

        m_descriptorSetLayout = Application::GetGfxApp()->CreateDescriptorSetLayout(std::move(layoutInfos));
        m_descriptorSet = Application::GetGfxApp()->GetDescriptorManager()->GetDescriptorSet(m_descriptorSetLayout.get());

        m_descriptorSet->AddDescriptor("Camera", 0);
        m_descriptorSet->AddDescriptor("Lighting", 1);
        m_descriptorSet->AddDescriptor("ShaderParameter", 2);

    }
    void Material::EndGpu()
    {

    }
    void Material::SetInt(const string& name, int value)
    {

    }
    void Material::SetFloat(const string& name, float value)
    {
        m_isDirtyParameter = true;
        m_parameterValues[name].SetValue(value);
        //m_descriptorSet->Find("ShaderParameter")->IsDirty = true;
    }

    void Material::SetColor(const string& name, const Color4f& value)
    {
        m_isDirtyParameter = true;
        m_parameterValues[name].SetValue(value);
    }

    void Material::SetTexture(const string& name, Texture_ref value)
    {
        m_isDirtyParameter = true;
        m_parameterValues[name].SetValue(value);
    }
    void Material::SetMatrix4f(const string& name, const Matrix4f& value)
    {
        m_isDirtyParameter = true;
        m_parameterValues[name].SetValue(value);
    }
    void Material::SetVector4(const string& name, const Vector4f& value)
    {
        m_isDirtyParameter = true;
        m_parameterValues[name].SetValue(value);
    }
    int Material::GetInt(const string& name)
    {
        auto it = m_parameterValues.find(name);
        if (it == m_parameterValues.end())
        {
            return 0;
        }
        return it->second.AsInt();
    }
    float Material::GetFloat(const string& name)
    {
        auto it = m_parameterValues.find(name);
        if (it == m_parameterValues.end())
        {
            return 0;
        }
        return it->second.AsFloat();
    }
    Color4f Material::GetColor(const string& name)
    {
        auto it = m_parameterValues.find(name);
        if (it == m_parameterValues.end())
        {
            return Color4f{};
        }
        return it->second.AsColor();
    }
    Vector4f Material::GetVector4(const string& name)
    {
        auto it = m_parameterValues.find(name);
        if (it == m_parameterValues.end())
        {
            return Vector4f{};
        }
        return it->second.AsVector();
    }
    Texture_ref Material::GetTexture(const string& name)
    {
        auto it = m_parameterValues.find(name);
        if (it == m_parameterValues.end())
        {
            return Texture_ref{};
        }
        return it->second.AsTexture();

    }
    Matrix4f Material::GetMatrix4f(const string& name)
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


        m_descriptorSet->Find("ShaderParameter")->SetConstantBuffer(m_buffer.get());

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
}