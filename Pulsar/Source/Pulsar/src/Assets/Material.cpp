#include "Assets/Material.h"
#include "Application.h"
#include "Assets/StaticMesh.h"
#include "Assets/Texture2D.h"
#include "BuiltinAsset.h"
#include "Logger.h"

#include <CoreLib.Serialization/JsonSerializer.h>
#include <Pulsar/AssetManager.h>
#include <Pulsar/Assets/Material.h>
#include <mutex>

namespace pulsar
{

    ObjectPtr<Material> Material::StaticCreate(string_view name)
    {
        Material_sp material = mksptr(new Material);
        material->Construct();
        material->SetName(name);

        return material;
    }

    void Material::OnConstruct()
    {
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
            ObjectPtr<Texture> tex = ObjectHandle::parse(value);
            ret.SetValue(tex);
            break;
        }
        }

        return ret;
    }

    bool Material::CreateGPUResource()
    {
        if (m_createdGpuResource)
        {
            return true;
        }

        Shader_ref shader = m_shader;
        if (!m_shader || !m_shader->m_isAvailable)
        {
            shader = GetAssetManager()->LoadAsset<Shader>(BuiltinAsset::Shader_Missing);
        }

        m_createdGpuResource = true;

        const auto& passes = shader->GetSourceData().ApiMaps.at(Application::GetGfxApp()->GetApiType());
        auto shaderConfig = shader->GetConfig();

        // process deferred
        // create shader module from source
        gfx::GFXGpuProgram_sp gpuProgram = Application::GetGfxApp()->CreateGpuProgram(passes.Sources);

        // create shader pass state config
        gfx::GFXShaderPassConfig config{};
        {
            config.CullMode = shaderConfig->CullMode;
            config.DepthCompareOp = shaderConfig->DepthCompareOp;
            config.DepthTestEnable = shaderConfig->DepthTestEnable;
            config.DepthWriteEnable = shaderConfig->DepthWriteEnable;
            config.StencilTestEnable = shaderConfig->StencilTestEnable;
        }

        // create shader pass
        m_gfxShaderPasses = Application::GetGfxApp()->CreateShaderPass(
            config,
            gpuProgram);

        // create constant layouts and constant buffer
        array_list<gfx::GFXDescriptorSetLayoutInfo> descLayoutInfos;

        // create constant buffer
        if (m_constantBufferSize)
        {
            m_materialConstantBuffer = Application::GetGfxApp()->CreateBuffer(gfx::GFXBufferUsage::ConstantBuffer, m_constantBufferSize);
        }

        if (m_constantBufferSize != 0)
        {
            auto info = gfx::GFXDescriptorSetLayoutInfo{
                gfx::GFXDescriptorType::ConstantBuffer,
                gfx::GFXShaderStageFlags::VertexFragment,
                0, 3};
            descLayoutInfos.push_back(info);
        }

        // create texture parameter layout
        if (shaderConfig->Properties)
        {
            auto offset = m_constantBufferSize != 0 ? 1 : 0;
            const auto count = shaderConfig->Properties->size();
            for (size_t i = 0; i < count; ++i)
            {
                auto& item = shaderConfig->Properties->at(i);
                auto info = gfx::GFXDescriptorSetLayoutInfo{
                    gfx::GFXDescriptorType::CombinedImageSampler,
                    gfx::GFXShaderStageFlags::VertexFragment,
                    (uint32_t)i + offset, 3};
                descLayoutInfos.push_back(info);
            }
        }

        // create descriptorset layouts
        m_descriptorSetLayout = Application::GetGfxApp()->CreateDescriptorSetLayout(
            descLayoutInfos.data(),
            descLayoutInfos.size());

        // create descriptor set
        m_descriptorSet = Application::GetGfxApp()->GetDescriptorManager()->GetDescriptorSet(m_descriptorSetLayout);
        if (m_constantBufferSize)
        {
            m_descriptorSet->AddDescriptor("ConstantProperties", 0)->SetConstantBuffer(m_materialConstantBuffer.get());
        }
        if (shaderConfig->Properties && !shaderConfig->Properties->empty())
        {
            auto count = shaderConfig->Properties->size();
            auto offset = m_constantBufferSize ? 1 : 0;
            for (int i = 0; i < count; ++i)
            {
                auto item = shaderConfig->Properties->at(i);
                auto desc = m_descriptorSet->AddDescriptor(item->Name, i + offset);
            }
        }

        m_isDirtyParameter = true;
        SubmitParameters();
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
        m_descriptorSetLayout.reset();
        m_materialConstantBuffer.reset();
    }
    bool Material::IsCreatedGPUResource() const
    {
        return m_createdGpuResource;
    }

    static ser::VarientRef NewVectorObject(const ser::VarientRef& ctx, Vector4f vec)
    {
        auto obj = ctx->New(ser::VarientType::Object);
        obj->Add("x", vec.x);
        obj->Add("y", vec.y);
        obj->Add("z", vec.z);
        obj->Add("w", vec.w);
        return obj;
    }
    static Vector4f GetVectorObject(const ser::VarientRef& var)
    {
        float x = var->At("x")->AsFloat();
        float y = var->At("y")->AsFloat();
        float z = var->At("z")->AsFloat();
        float w = var->At("w")->AsFloat();
        return {x, y, z, w};
    }

    void Material::Serialize(AssetSerializer* s)
    {
        if (s->IsWrite)
        {
            const auto shaderObject = s->Object->New(ser::VarientType::String);
            shaderObject->Assign(m_shader.handle.to_string());
            s->Object->Add("Shader", shaderObject);

            const auto parametersArray = s->Object->New(ser::VarientType::Array);
            for (auto& [name, value] : m_parameterValues)
            {
                ser::VarientRef parameter = parametersArray->New(ser::VarientType::Object);
                parameter->Add("Name", name.to_string());
                parameter->Add("Type", mkbox(value.Type)->GetName());
                ser::VarientRef paramValue;
                switch (value.Type)
                {
                case ShaderParameterType::IntScalar:
                    paramValue = parameter->New(ser::VarientType::Number)->Assign(value.AsIntScalar());
                    break;
                case ShaderParameterType::Scalar:
                    paramValue = parameter->New(ser::VarientType::Number)->Assign(value.AsScalar());
                    break;
                case ShaderParameterType::Vector:
                    paramValue = parameter->New(ser::VarientType::Object);
                    paramValue->Assign(NewVectorObject(parameter, value.AsVector()));
                    break;
                case ShaderParameterType::Texture2D:
                    paramValue = parameter->New(ser::VarientType::String);
                    paramValue->Assign(value.AsTexture().GetHandle().to_string());
                    break;
                default:;
                }
                parameter->Add("Value", paramValue);
                parametersArray->Push(parameter);
            }
            s->Object->Add("Parameters", parametersArray);
        }
        else
        {
            // parameters
            auto parameterObject = s->Object->At("Parameters");
            auto parametersCount = parameterObject->GetCount();
            for (int i = 0; i < parametersCount; ++i)
            {
                auto parameter = parameterObject->At(i);
                auto name = parameter->At("Name")->AsString();
                auto typestr = parameter->At("Type")->AsString();
                auto valueObject = parameter->At("Value");

                uint32_t typenum{};

                Enum::StaticTryParse(cltypeof<BoxingShaderParameterType>(), typestr, &typenum);
                ShaderParameterType type = static_cast<ShaderParameterType>(typenum);

                MaterialParameterValue paramValue{};
                switch (type)
                {
                case ShaderParameterType::IntScalar:
                    paramValue.SetValue(valueObject->AsInt());
                    break;
                case ShaderParameterType::Scalar:
                    paramValue.SetValue(valueObject->AsFloat());
                    break;
                case ShaderParameterType::Vector:
                    paramValue.SetValue(GetVectorObject(valueObject));
                    break;
                case ShaderParameterType::Texture2D: {
                    Texture_ref tex = ObjectHandle::parse(valueObject->AsString());
                    paramValue.SetValue(tex);
                    break;
                }
                default:;
                }
                m_parameterValues.insert({index_string{name}, paramValue});
            }

            // shader
            auto shaderObject = ObjectHandle::parse(s->Object->At("Shader")->AsString());
            TryFindOrLoadObject(shaderObject);
            SetShader(shaderObject);
        }
    }

    void Material::SetIntScalar(const index_string& name, int value)
    {
        if (!m_parameterValues.contains(name))
        {
            return;
        }
        m_parameterValues[name].SetValue(value);
        m_isDirtyParameter = true;
    }
    void Material::SetFloat(const index_string& name, float value)
    {
        if (!m_parameterValues.contains(name))
        {
            return;
        }
        m_parameterValues[name].SetValue(value);
        m_isDirtyParameter = true;
    }

    void Material::SetTexture(const index_string& name, Texture_ref value)
    {
        if (!m_parameterValues.contains(name))
        {
            return;
        }
        m_parameterValues[name].SetValue(value);
        m_isDirtyParameter = true;
    }

    void Material::SetVector4(const index_string& name, const Vector4f& value)
    {
        if (!m_parameterValues.contains(name))
        {
            return;
        }
        m_parameterValues[name].SetValue(value);
        m_isDirtyParameter = true;
    }
    int Material::GetIntScalar(const index_string& name)
    {
        auto it = m_parameterValues.find(name);
        if (it == m_parameterValues.end())
        {
            auto meta = m_propertyInfo.find(name);
            if (meta != m_propertyInfo.end())
            {
                return meta->second.Value.AsIntScalar();
            }
            return 0;
        }
        return it->second.AsIntScalar();
    }

    float Material::GetFloat(const index_string& name)
    {
        auto it = m_parameterValues.find(name);
        if (it == m_parameterValues.end())
        {
            auto meta = m_propertyInfo.find(name);
            if (meta != m_propertyInfo.end())
            {
                return meta->second.Value.AsScalar();
            }
            return 0;
        }
        return it->second.AsScalar();
    }

    Vector4f Material::GetVector4(const index_string& name)
    {
        auto it = m_parameterValues.find(name);
        if (it == m_parameterValues.end())
        {
            auto meta = m_propertyInfo.find(name);
            if (meta != m_propertyInfo.end())
            {
                return meta->second.Value.AsVector();
            }
            return Vector4f{};
        }
        return it->second.AsVector();
    }

    Texture_ref Material::GetTexture(const index_string& name)
    {
        auto it = m_parameterValues.find(name);
        if (it == m_parameterValues.end())
        {
            auto meta = m_propertyInfo.find(name);
            if (meta != m_propertyInfo.end())
            {
                return meta->second.Value.AsTexture();
            }
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
        if (m_materialConstantBuffer == nullptr)
        {
            return;
        }

        m_bufferData.resize(m_materialConstantBuffer->GetSize());
        for (auto& [name, value] : m_parameterValues)
        {
            auto offset = m_propertyInfo[name].Offset;
            auto ptr = m_bufferData.data() + offset;
            switch (value.Type)
            {
            case ShaderParameterType::IntScalar: {
                *(int*)ptr = value.AsIntScalar();
                break;
            }
            case ShaderParameterType::Scalar: {
                *(float*)ptr = value.AsScalar();
                break;
            }
            case ShaderParameterType::Vector: {
                *(Vector4f*)ptr = value.AsVector();
                break;
            }
            case ShaderParameterType::Texture2D: {
                Texture2D_ref tex = value.AsTexture();
                m_descriptorSet->Find(name.to_string())->SetTextureSampler2D(tex->GetGFXTexture().get());
                break;
            }
            default:;
            }
        }

        m_materialConstantBuffer->Fill(m_bufferData.data());

        m_descriptorSet->Find("ConstantProperties")->SetConstantBuffer(m_materialConstantBuffer.get());

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
    void Material::OnShaderAvailable()
    {
        bool b = m_shader->m_isAvailable;
        if (b)
        {
            bool created = m_createdGpuResource;
            if (created)
            {
                DestroyGPUResource();
            }
            InitShaderConfig();
            if (created)
            {
                CreateGPUResource();
            }
        }
        else
        {
        }
    }

    void Material::InitShaderConfig()
    {
        if (!m_shader->HasSupportedApiData(Application::GetGfxApp()->GetApiType()))
        {
            return;
        }
        auto config = m_shader->GetConfig();

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

        // create parameter default value
        for (auto& prop : m_propertyInfo)
        {
            if (!m_parameterValues.contains(prop.first))
            {
                m_parameterValues.insert({prop.first, prop.second.Value});
            }
        }
    }

    Shader_ref Material::GetShader() const
    {
        return m_shader;
    }

    void Material::SetShader(Shader_ref value)
    {
        if (m_shader)
        {
            m_shader->OnAvailableChanged.RemoveListener(this, &ThisClass::OnShaderAvailable);
        }
        m_shader = value;
        if (m_shader)
        {
            std::lock_guard<std::mutex> lk{m_shader->m_isAvailableMutex};
            if (m_shader->m_isAvailable)
            {
                InitShaderConfig();
            }
            m_shader->OnAvailableChanged.AddListener(this, &ThisClass::OnShaderAvailable);
        }
        if (m_createdGpuResource)
        {
            DestroyGPUResource();
            CreateGPUResource();
        }
        OnShaderChanged.Invoke();
    }

    gfx::GFXShaderPass_sp Material::GetGfxShaderPass()
    {
        return m_gfxShaderPasses;
    }

} // namespace pulsar