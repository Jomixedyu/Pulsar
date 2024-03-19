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
#include <utility>

namespace pulsar
{

    RCPtr<Material> Material::StaticCreate(string_view name)
    {
        Material_sp material = mksptr(new Material);
        material->Construct();
        material->SetName(name);
        material->SetShader(GetAssetManager()->LoadAsset<Shader>(BuiltinAsset::Shader_Missing));

        return material.get();
    }

    void Material::OnConstruct()
    {
    }
    void Material::ClearUnusedParameterValue()
    {
        for (auto& name : m_shader->GetPropertyNames())
        {
            m_parameterValues.erase(name);
        }
    }

    bool Material::CreateGPUResource()
    {
        if (m_createdGpuResource)
        {
            return true;
        }

        RCPtr<Shader> shader = m_shader;
        TryLoadAssetRCPtr(shader);
        assert(shader);
        // if (!m_shader)
        // {
        //     shader = GetAssetManager()->LoadAsset<Shader>(BuiltinAsset::Shader_Missing);
        // }

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

        const auto cbufferSize = m_shader->GetConstantBufferSize();

        // create constant buffer
        if (cbufferSize)
        {
            m_materialConstantBuffer = Application::GetGfxApp()->CreateBuffer(gfx::GFXBufferUsage::ConstantBuffer, cbufferSize);
        }

        if (cbufferSize != 0)
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
            auto offset = cbufferSize != 0 ? 1 : 0;
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
        if (cbufferSize)
        {
            m_descriptorSet->AddDescriptor("ConstantProperties", 0)->SetConstantBuffer(m_materialConstantBuffer.get());
        }
        if (shaderConfig->Properties && !shaderConfig->Properties->empty())
        {
            const auto count = shaderConfig->Properties->size();
            const auto offset = cbufferSize ? 1 : 0;
            for (int i = 0; i < count; ++i)
            {
                const auto item = shaderConfig->Properties->at(i);
                m_descriptorSet->AddDescriptor(item->Name, i + offset);
            }
        }


        SubmitParameters(true);
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
    void Material::OnDependencyMessage(ObjectHandle inDependency, DependencyObjectState msg)
    {
        base::OnDependencyMessage(inDependency, msg);
        if (EnumHasFlag(msg, DependencyObjectState::Reload))
        {
            SetShader(inDependency);
        }
        else if (EnumHasFlag(msg, DependencyObjectState::Unload))
        {
            SetShader(nullptr);
        }
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
            shaderObject->Assign(m_shader.GetHandle().to_string());
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
        else //read
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
                    RCPtr<Texture2D> tex = ObjectHandle::parse(valueObject->AsString());
                    // RCPtr<Texture2D> tex = GetAssetManager()->LoadAssetById(handle);
                    paramValue.SetValue(tex);
                    break;
                }
                default:;
                }
                m_parameterValues.insert({index_string{name}, paramValue});
            }

            // shader
            auto shaderObject = ObjectHandle::parse(s->Object->At("Shader")->AsString());
            auto shader = GetAssetManager()->LoadAssetById(shaderObject);
            SetShader(shader);
        }
    }

    void Material::OnInstantiateAsset(AssetObject* obj)
    {
        base::OnInstantiateAsset(obj);
        auto self = static_cast<Material*>(obj);
        self->m_parameterValues = m_parameterValues;
        self->SetShader(m_shader);
    }


    #pragma region MaterialParameters
    void Material::SetIntScalar(const index_string& name, int value)
    {
        m_parameterValues[name].SetValue(value);
        m_isDirtyParameter = true;
    }
    void Material::SetFloat(const index_string& name, float value)
    {
        m_parameterValues[name].SetValue(value);
        m_isDirtyParameter = true;
    }

    void Material::SetTexture(const index_string& name, const RCPtr<Texture2D>& value)
    {
        m_parameterValues[name].SetValue(value);
        m_isDirtyParameter = true;
    }

    void Material::SetVector4(const index_string& name, const Vector4f& value)
    {
        m_parameterValues[name].SetValue(value);
        m_isDirtyParameter = true;
    }
    int Material::GetIntScalar(const index_string& name)
    {
        auto it = m_parameterValues.find(name);
        if (it == m_parameterValues.end())
        {
            if (auto prop = m_shader->GetPropertyInfo(name))
            {
                return prop->Value.AsIntScalar();
            }
            return 0;
        }
        return it->second.AsIntScalar();
    }

    float Material::GetScalar(const index_string& name)
    {
        auto it = m_parameterValues.find(name);
        if (it == m_parameterValues.end())
        {
            if (auto prop = m_shader->GetPropertyInfo(name))
            {
                return prop->Value.AsScalar();
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
            if (auto prop = m_shader->GetPropertyInfo(name))
            {
                return prop->Value.AsVector();
            }
            return Vector4f{};
        }
        return it->second.AsVector();
    }

    RCPtr<Texture2D> Material::GetTexture(const index_string& name)
    {
        auto it = m_parameterValues.find(name);
        if (it == m_parameterValues.end())
        {
            if (auto prop = m_shader->GetPropertyInfo(name))
            {
                return prop->Value.AsTexture();
            }
            return {};
        }
        return it->second.AsTexture();
    }
    #pragma endregion


    void Material::SubmitParameters(bool force)
    {
        if (!m_isDirtyParameter && !force)
        {
            return;
        }
        const auto cbufferSize = m_shader->GetConstantBufferSize();
        if (cbufferSize != 0 && m_materialConstantBuffer == nullptr)
        {
            return;
        }

        m_bufferData.resize(cbufferSize);

        //assign buffer and descriptor
        for (auto& name : m_shader->GetPropertyNames())
        {
            auto& prop = *m_shader->GetPropertyInfo(name);

            auto offset = prop.Offset;
            auto ptr = m_bufferData.data() + offset;

            const MaterialParameterValue* paramValue;
            bool isUseDefault = false;
            if (m_parameterValues.contains(name))
            {
                paramValue = &m_parameterValues[name];
            }
            else
            {
                paramValue = &prop.Value;
                isUseDefault = true;
            }

            switch (paramValue->Type)
            {
            case ShaderParameterType::IntScalar: {
                *(int*)ptr = paramValue->AsIntScalar();
                break;
            }
            case ShaderParameterType::Scalar: {
                *(float*)ptr = paramValue->AsScalar();
                break;
            }
            case ShaderParameterType::Vector: {
                *(Vector4f*)ptr = paramValue->AsVector();
                break;
            }
            case ShaderParameterType::Texture2D: {
                if (!isUseDefault)
                {
                    auto t = paramValue->AsTexture();
                    TryLoadAssetRCPtr(t);
                    if (!t)
                    {
                        paramValue = &prop.Value;
                        isUseDefault = true;
                    }
                }
                auto tex = paramValue->AsTexture();
                TryLoadAssetRCPtr(tex);
                tex->CreateGPUResource();
                m_descriptorSet->Find(name.to_string())->SetTextureSampler2D(tex->GetGFXTexture().get());
                break;
            }
            default:
                assert(false);
                break;
            }
        }

        if (cbufferSize > 0)
        {
            m_materialConstantBuffer->Fill(m_bufferData.data());
            m_descriptorSet->Find("ConstantProperties")->SetConstantBuffer(m_materialConstantBuffer.get());
        }


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

    RCPtr<Shader> Material::GetShader() const
    {
        return m_shader;
    }

    void Material::SetShader(RCPtr<Shader> value)
    {
        if (m_shader)
        {
            RuntimeObjectManager::RemoveDependList(GetObjectHandle(), m_shader->GetObjectHandle());
        }

        m_shader = std::move(value);
        if (m_shader == nullptr)
        {
            m_shader = GetAssetManager()->LoadAsset<Shader>(BuiltinAsset::Shader_Missing);
        }
        RuntimeObjectManager::AddDependList(GetObjectHandle(), m_shader->GetObjectHandle());

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