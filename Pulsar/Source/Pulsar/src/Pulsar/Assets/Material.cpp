#include "Assets/Material.h"
#include "Application.h"
#include "AssetSerializerUtil.h"
#include "Assets/Texture2D.h"
#include "BuiltinAsset.h"
#include "Logger.h"

#include <CoreLib.Serialization/JsonSerializer.h>
#include <Pulsar/AssetManager.h>
#include <Pulsar/Assets/Material.h>
#include <Pulsar/Rendering/ShaderInstanceCache.h>
#include <Pulsar/Rendering/ShaderPropertySync.h>
#include <mutex>
#include <utility>

namespace pulsar
{

    RCPtr<Material> Material::StaticCreate(const RCPtr<Shader>& shader, string_view name)
    {
        auto material = NewAssetObject<Material>();
        material->SetName(name);
        auto s = shader;
        if (!s)
        {
            s = AssetManager::Get()->LoadAsset<Shader>(BuiltinAsset::Shader_Missing);
        }
        material->SetShader(s);
        return material;
    }

    bool Material::CreateGPUResource()
    {
        if (m_createdGpuResource)
        {
            return true;
        }
        m_createdGpuResource = true;
        // 轻量初始化: GPU 资源将在第一个 ShaderInstance Ready 后通过 EnsureGPUResources 懒创建
        return true;
    }

    void Material::DestroyGPUResource()
    {
        if (!m_createdGpuResource)
        {
            return;
        }
        m_createdGpuResource = false;
        m_gpuResourcesInitialized = false;
        m_passBindings.clear();
        m_descriptorSet.reset();
        m_descriptorSetLayout.reset();
        m_materialConstantBuffer.reset();
    }

    bool Material::IsCreatedGPUResource() const
    {
        return m_createdGpuResource;
    }

    void Material::GetSubscribeObserverHandles(array_list<ObjectHandle>& out)
    {
        base::GetSubscribeObserverHandles(out);
        out.push_back(m_shader.GetHandle());

        // 收集 Sheet 中的纹理引用
        for (const auto& [name, value] : m_sheet.GetAllProperties())
        {
            RCPtr<Texture> tex;
            if (m_sheet.GetTexture(name, tex) && tex)
            {
                out.push_back(tex.GetHandle());
            }
        }
    }

    void Material::OnNotifyObserver(ObjectHandle inDependency, DependencyObjectState msg)
    {
        base::OnNotifyObserver(inDependency, msg);
        if (inDependency == m_shader.GetHandle())
        {
            if (EnumHasFlag(msg, DependencyObjectState::Modified))
            {
                // Shader 修改 -> 清除绑定，等待重新获取
                ClearPassBindings();
            }
            else if (EnumHasFlag(msg, DependencyObjectState::Unload))
            {
                ClearPassBindings();
            }
        }
        SubmitParameters(true);
    }

    void Material::Serialize(AssetSerializer* s)
    {
        if (s->IsWrite)
        {
            const auto shaderObject = s->Object->New(ser::VarientType::String);
            shaderObject->Assign(m_shader.GetGuid().to_string());
            s->Object->Add("Shader", shaderObject);

            // 序列化 activeFeatures
            const auto featuresArray = s->Object->New(ser::VarientType::Array);
            for (const auto& feature : m_activeFeatures)
            {
                auto featureObj = featuresArray->New(ser::VarientType::String);
                featureObj->Assign(feature);
                featuresArray->Push(featureObj);
            }
            s->Object->Add("Features", featuresArray);

            // 序列化参数 (m_sheet)
            const auto parametersArray = s->Object->New(ser::VarientType::Array);
            for (const auto& [name, value] : m_sheet.GetAllProperties())
            {
                ser::VarientRef parameter = parametersArray->New(ser::VarientType::Object);
                parameter->Add("Name", name);

                // 使用新枚举名
                parameter->Add("Type", mkbox(value.GetType())->GetName());

                ser::VarientRef paramValue;
                switch (value.GetType())
                {
                case ShaderPropertyType::Int:
                    paramValue = parameter->New(ser::VarientType::Number)->Assign(value.AsInt());
                    break;
                case ShaderPropertyType::Float:
                    paramValue = parameter->New(ser::VarientType::Number)->Assign(value.AsFloat());
                    break;
                case ShaderPropertyType::Float4:
                    paramValue = parameter->New(ser::VarientType::Object);
                    paramValue->Assign(AssetSerializerUtil::NewObject(parameter, value.AsFloat4()));
                    break;
                case ShaderPropertyType::Texture2D: {
                    paramValue = parameter->New(ser::VarientType::String);
                    auto tex = value.AsTexture2D();
                    paramValue->Assign(tex ? tex->GetAssetGuid().to_string() : "");
                    break;
                }
                default:
                    break;
                }
                parameter->Add("Value", paramValue);
                parametersArray->Push(parameter);
            }
            s->Object->Add("Parameters", parametersArray);
        }
        else // read
        {
            // 读取 features
            if (auto featuresObj = s->Object->At("Features"))
            {
                auto count = featuresObj->GetCount();
                for (int i = 0; i < count; ++i)
                {
                    m_activeFeatures.push_back(featuresObj->At(i)->AsString());
                }
            }

            // 读取参数
            if (auto parameterObject = s->Object->At("Parameters"))
            {
                auto parametersCount = parameterObject->GetCount();
                for (int i = 0; i < parametersCount; ++i)
                {
                    auto parameter = parameterObject->At(i);
                    auto name = parameter->At("Name")->AsString();
                    auto typestr = parameter->At("Type")->AsString();
                    auto valueObject = parameter->At("Value");

                    uint32_t typenum{};
                    Enum::StaticTryParse(cltypeof<BoxingShaderPropertyType>(), typestr, &typenum);
                    ShaderPropertyType type = static_cast<ShaderPropertyType>(typenum);

                    switch (type)
                    {
                    case ShaderPropertyType::Int:
                        m_sheet.SetInt(name, valueObject->AsInt());
                        break;
                    case ShaderPropertyType::Float:
                        m_sheet.SetFloat(name, valueObject->AsFloat());
                        break;
                    case ShaderPropertyType::Float4:
                        m_sheet.SetFloat4(name, AssetSerializerUtil::GetVector4Object(valueObject));
                        break;
                    case ShaderPropertyType::Texture2D: {
                        guid_t guid = guid_t::parse(valueObject->AsString());
                        RCPtr<Texture2D> tex = RuntimeAssetManager::GetLoadedAssetByGuid<Texture2D>(guid);
                        m_sheet.SetTexture(name, tex);
                        break;
                    }
                    default:
                        break;
                    }
                }
            }

            // 读取 shader
            auto shaderObject = guid_t::parse(s->Object->At("Shader")->AsString());
            auto shader = RuntimeAssetManager::GetLoadedAssetByGuid<Shader>(shaderObject);
            SetShader(shader);

            RuntimeObjectManager::RebuildMessageBox(this);
        }
    }

    void Material::OnInstantiateAsset(AssetObject* obj)
    {
        base::OnInstantiateAsset(obj);
        auto self = static_cast<Material*>(obj);
        self->m_sheet = m_sheet;
        self->m_activeFeatures = m_activeFeatures;
        self->SetShader(m_shader);
    }

#pragma region MaterialParameters
    void Material::SetIntScalar(const index_string& name, int value)
    {
        m_sheet.SetInt(name.to_string(), value);
        m_isDirtyParameter = true;
    }

    void Material::SetFloat(const index_string& name, float value)
    {
        m_sheet.SetFloat(name.to_string(), value);
        m_isDirtyParameter = true;
    }

    void Material::SetTexture(const index_string& name, const RCPtr<Texture>& value)
    {
        m_sheet.SetTexture(name.to_string(), value);
        m_isDirtyParameter = true;
        RuntimeObjectManager::RebuildMessageBox(this);
    }

    void Material::SetVector4(const index_string& name, const Vector4f& value)
    {
        m_sheet.SetFloat4(name.to_string(), value);
        m_isDirtyParameter = true;
    }

    // 从 ShaderConfig::Properties 中查找指定名称的 DefaultValue
    static const ShaderConfigProperty* FindConfigProperty(const Shader* shader, const std::string& propName)
    {
        if (!shader) return nullptr;
        auto config = shader->GetConfig();
        if (!config || !config->Properties) return nullptr;
        for (const auto& prop : *config->Properties)
        {
            if (prop->Name == propName)
                return prop.get();
        }
        return nullptr;
    }

    int Material::GetIntScalar(const index_string& name)
    {
        int val = 0;
        if (m_sheet.GetInt(name.to_string(), val))
            return val;
        if (auto configProp = FindConfigProperty(m_shader.GetPtr(), name.to_string()))
        {
            if (!configProp->DefaultValue.empty())
                return std::atoi(configProp->DefaultValue.c_str());
        }
        return 0;
    }

    float Material::GetScalar(const index_string& name)
    {
        float val = 0;
        if (m_sheet.GetFloat(name.to_string(), val))
            return val;
        if (auto configProp = FindConfigProperty(m_shader.GetPtr(), name.to_string()))
        {
            if (!configProp->DefaultValue.empty())
                return static_cast<float>(std::atof(configProp->DefaultValue.c_str()));
        }
        return 0;
    }

    Vector4f Material::GetVector4(const index_string& name)
    {
        Vector4f val{};
        if (m_sheet.GetFloat4(name.to_string(), val))
            return val;
        // 也尝试 Color 类型（Color4f 和 Vector4f 内存布局相同）
        Color4f col{};
        if (m_sheet.GetColor(name.to_string(), col))
            return {col.r, col.g, col.b, col.a};
        return {};
    }

    RCPtr<Texture> Material::GetTexture(const index_string& name)
    {
        RCPtr<Texture> tex;
        if (m_sheet.GetTexture(name.to_string(), tex))
            return tex;
        return {};
    }
#pragma endregion

    void Material::SubmitParameters(bool force)
    {
        if (!IsCreatedGPUResource())
            return;

        // Detect shader program change (e.g. async compilation finished), rebuild GPU resources if needed
        for (const auto& [key, binding] : m_passBindings)
        {
            auto program = binding.GetCurrentProgram();
            if (!program) continue;
            if (program != m_builtWithProgram.lock())
            {
                m_descriptorSet.reset();
                m_descriptorSetLayout.reset();
                m_materialConstantBuffer.reset();
                m_gpuResourcesInitialized = false;
                EnsureGPUResources(program->m_layout);
                m_builtWithProgram = program;
                m_isDirtyParameter = true; // force re-upload parameters with new layout
            }
            break; // all passes share the same GPU resources, only check the first
        }

        if (!m_gpuResourcesInitialized)
            return;

        // 确保 shader 默认值已写入 sheet（幂等，不覆盖用户已设置的值）
        ApplyShaderDefaults();

        if (!m_isDirtyParameter && !force)
            return;

        // 获取第一个 ready 的 binding 的 layout，同步到 GPU
        for (const auto& [key, binding] : m_passBindings)
        {
            auto program = binding.GetCurrentProgram();
            if (program)
            {
                ShaderPropertySync::SyncSheetToGpu(
                    m_sheet,
                    program->m_layout,
                    m_materialConstantBuffer.get(),
                    m_descriptorSet.get());
                break;
            }
        }

        if (m_descriptorSet)
        {
            m_descriptorSet->Submit();
        }
        m_isDirtyParameter = false;
    }

    const MaterialPassBinding& Material::GetPassBinding(
        const std::string& passName,
        const std::string& interface_)
    {
        PassKey key{passName, interface_};
        auto it = m_passBindings.find(key);
        if (it != m_passBindings.end())
            return it->second;

        // 懒创建: 从 ShaderInstanceCache 获取
        if (!m_shader)
        {
            static MaterialPassBinding empty{};
            return empty;
        }

        auto config = m_shader->GetConfig();

        ShaderVariantKey variantKey;
        variantKey.m_shaderGuid = m_shader->GetAssetGuid();
        variantKey.m_passName = passName;
        variantKey.m_interface = interface_;
        variantKey.m_features = m_activeFeatures;

        // 构造编译任务 (路径由编译器服务根据 guid 解析)
        ShaderCompileTask task;
        task.m_variantKey = variantKey;

        // 入口点: 从 ShaderConfig 的 Passes 中查找
        if (config && config->Passes)
        {
            // 优先匹配 passName
            for (const auto& pass : *config->Passes)
            {
                if (pass->Name == passName && pass->Entry)
                {
                    task.m_entries.m_vertex = pass->Entry->Vertex;
                    task.m_entries.m_fragment = pass->Entry->Fragment;
                    task.m_entries.m_tessControl = pass->Entry->TessControl;
                    task.m_entries.m_tessEval = pass->Entry->TessEval;
                    break;
                }
            }

            // fallback: 如果没找到匹配的 pass，使用第一个 pass 的 entry
            if (task.m_entries.m_vertex.empty() && task.m_entries.m_fragment.empty())
            {
                for (const auto& pass : *config->Passes)
                {
                    if (pass->Entry)
                    {
                        task.m_entries.m_vertex = pass->Entry->Vertex;
                        task.m_entries.m_fragment = pass->Entry->Fragment;
                        task.m_entries.m_tessControl = pass->Entry->TessControl;
                        task.m_entries.m_tessEval = pass->Entry->TessEval;
                        Logger::Log("Material pass '" + passName + "' not found, using first pass '" + pass->Name + "' entry as fallback", LogLevel::Warning);
                        break;
                    }
                }
            }
        }

        auto instance = ShaderInstanceCache::Instance().GetOrCreate(variantKey, task);
        MaterialPassBinding binding;
        binding.m_instance = instance;
        auto [insertIt, _] = m_passBindings.emplace(key, std::move(binding));

        return insertIt->second;
    }

    void Material::EnsureGPUResources(const ShaderPropertyLayout& layout)
    {
        if (m_gpuResourcesInitialized)
            return;

        auto gfxApp = Application::GetGfxApp();

        // 创建 descriptor set layout (set 0)
        array_list<gfx::GFXDescriptorSetLayoutDesc> descLayoutInfos;

        if (layout.m_totalCBufferSize > 0)
        {
            gfx::GFXDescriptorSetLayoutDesc cbDesc{};
            cbDesc.Type = gfx::GFXDescriptorType::ConstantBuffer;
            cbDesc.Stage = (layout.m_cbufferStageFlags != gfx::GFXGpuProgramStageFlags::None)
                ? layout.m_cbufferStageFlags
                : gfx::GFXGpuProgramStageFlags::VertexFragment;
            cbDesc.BindingPoint = layout.m_cbufferBindingPoint;
            descLayoutInfos.push_back(cbDesc);

            // 创建 cbuffer
            gfx::GFXBufferDesc bufferDesc{};
            bufferDesc.Usage = gfx::GFXBufferUsage::ConstantBuffer;
            bufferDesc.StorageType = gfx::GFXBufferMemoryPosition::VisibleOnDevice;
            bufferDesc.BufferSize = layout.m_totalCBufferSize;
            m_materialConstantBuffer = gfxApp->CreateBuffer(bufferDesc);
        }

        for (const auto& texEntry : layout.m_textureEntries)
        {
            gfx::GFXDescriptorSetLayoutDesc texDesc{};
            texDesc.Type = texEntry.m_isCombinedImageSampler
                ? gfx::GFXDescriptorType::CombinedImageSampler
                : gfx::GFXDescriptorType::Texture2D;
            texDesc.Stage = texEntry.m_stageFlags;
            texDesc.BindingPoint = texEntry.m_bindingPoint;
            descLayoutInfos.push_back(texDesc);
        }

        // 即使没有任何 binding 也创建空 layout，确保 set 0 始终存在以保证 set 编号对齐
        m_descriptorSetLayout = gfxApp->CreateDescriptorSetLayout(
            descLayoutInfos.data(), descLayoutInfos.size());
        m_descriptorSet = gfxApp->GetDescriptorManager()->GetDescriptorSet(m_descriptorSetLayout);

        if (m_materialConstantBuffer)
        {
            m_descriptorSet->AddDescriptor("ConstantProperties", 0)->SetConstantBuffer(m_materialConstantBuffer.get());
        }

        for (const auto& texEntry : layout.m_textureEntries)
        {
            m_descriptorSet->AddDescriptor(texEntry.m_name, texEntry.m_bindingPoint);
        }

        m_gpuResourcesInitialized = true;
    }

    void Material::ClearPassBindings()
    {
        m_passBindings.clear();
        m_gpuResourcesInitialized = false;
        m_descriptorSet.reset();
        m_descriptorSetLayout.reset();
        m_materialConstantBuffer.reset();
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

    void Material::ApplyShaderDefaults()
    {
        if (!m_shader) return;
        auto config = m_shader->GetConfig();
        if (!config || !config->Properties) return;

        auto assetMgr = AssetManager::Get() ? AssetManager::Get() : nullptr;

        for (const auto& prop : *config->Properties)
        {
            if (prop->DefaultValue.empty()) continue;

            // 只写入 sheet 里还没有的属性，不覆盖用户已设置的值
            switch (prop->Type)
            {
            case ShaderPropertyType::Int:
                if (!m_sheet.HasProperty(prop->Name))
                    m_sheet.SetInt(prop->Name, std::atoi(prop->DefaultValue.c_str()));
                break;
            case ShaderPropertyType::Float:
                if (!m_sheet.HasProperty(prop->Name))
                    m_sheet.SetFloat(prop->Name, static_cast<float>(std::atof(prop->DefaultValue.c_str())));
                break;
            case ShaderPropertyType::Float4:
            case ShaderPropertyType::Color:
            {
                if (!m_sheet.HasProperty(prop->Name))
                {
                    // 格式 "r,g,b,a"
                    float r = 0, g = 0, b = 0, a = 1;
                    sscanf_s(prop->DefaultValue.c_str(), "%f,%f,%f,%f", &r, &g, &b, &a);
                    if (prop->Type == ShaderPropertyType::Color)
                        m_sheet.SetColor(prop->Name, Color4f{r, g, b, a});
                    else
                        m_sheet.SetFloat4(prop->Name, Vector4f{r, g, b, a});
                }
                break;
            }
            case ShaderPropertyType::Texture2D:
            {
                if (!m_sheet.HasProperty(prop->Name) && assetMgr)
                {
                    const char* assetPath = BuiltinAsset::ResolveTextureDefault(prop->DefaultValue);
                    if (assetPath)
                    {
                        auto tex = assetMgr->LoadAsset<Texture>(assetPath);
                        m_sheet.SetTexture(prop->Name, tex);
                    }
                }
                break;
            }
            default:
                break;
            }
        }
    }

    void Material::SetShader(RCPtr<Shader> value)
    {
        m_shader = std::move(value);

        RuntimeObjectManager::RebuildMessageBox(this);

        ClearPassBindings();

        // shader 变更时将 DefaultValue 写进 sheet（不覆盖已有值）
        ApplyShaderDefaults();

        if (m_createdGpuResource)
        {
            DestroyGPUResource();
            CreateGPUResource();
        }

        OnShaderChanged.Invoke();
    }

    void Material::SetActiveFeatures(std::vector<std::string> features)
    {
        m_activeFeatures = std::move(features);
        ClearPassBindings();
    }
    void Material::OnCollectAssetDependencies(array_list<guid_t>& deps)
    {
        base::OnCollectAssetDependencies(deps);
        deps.push_back(m_shader.GetGuid());
    }

} // namespace pulsar