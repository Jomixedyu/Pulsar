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
#include <Pulsar/Rendering/RenderThread.h>
#include <gfx/GFXResourceManager.h>
#include <mutex>
#include <utility>

namespace pulsar
{

    Material::Material()
    {
        init_sptr_member(m_graphicsPipelineOverride);
        init_sptr_member(m_graphicsPipelineOverrideFields);
    }

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
        // 轻量初始化: GPU 资源将在 proxy 的 PrepareForRendering（渲染线程）于 shader Ready 后懒创建。
        // 游戏线程：从当前 sheet 解析渲染线程专用快照。
        ApplyShaderDefaults();
        RebuildRenderData();

        // 建渲染线程独占镜像，并喂入首份 shader + 参数快照（经队列，渲染线程帧首 drain 消费）。
        m_renderProxy = std::make_shared<MaterialProxy>();
        EnqueueProxyShaderUpdate();
        EnqueueProxyRenderData();
        return true;
    }

    void Material::DestroyGPUResource()
    {
        if (!m_createdGpuResource)
        {
            return;
        }
        m_createdGpuResource = false;

        // 让 proxy（连同其 descriptor set / cbuffer / layout 句柄）在渲染线程释放，
        // 避免 GPU 资源在游戏线程销毁、以及与渲染线程并发使用时的竞争。
        if (m_renderProxy)
        {
            if (auto* rt = Application::GetRenderThread())
            {
                rt->EnqueueDestroy_AnyThread(
                    [proxy = std::move(m_renderProxy)](gfx::GFXResourceManager*) mutable { proxy.reset(); });
            }
            m_renderProxy.reset();
        }
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
            if (EnumHasFlag(msg, DependencyObjectState::Modified) || EnumHasFlag(msg, DependencyObjectState::Unload))
            {
                // Shader 修改/卸载 -> 重新喂 shader 快照，proxy 清 bindings 等待重新获取
                EnqueueProxyShaderUpdate();
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

            // 序列化 Queue
            const auto queueObject = s->Object->New(ser::VarientType::String);
            queueObject->Assign(mkbox(m_queue)->GetName());
            s->Object->Add("Queue", queueObject);

            // 序列化 GraphicsPipelineOverride（只写被标记的字段）
            if (m_graphicsPipelineOverride && m_graphicsPipelineOverrideFields && !m_graphicsPipelineOverrideFields->IsEmpty())
            {
                const auto overrideObj = s->Object->New(ser::VarientType::Object);
                for (const auto& path : *m_graphicsPipelineOverrideFields->Paths)
                {
                    auto fieldName = path;
                    if (auto field = m_graphicsPipelineOverride->GetType()->GetFieldInfo(fieldName))
                    {
                        auto value = field->GetValue(m_graphicsPipelineOverride.get());
                        auto json = ser::JsonSerializer::Serialize(value.get(), {});
                        auto valueObj = overrideObj->New(ser::VarientType::Object);
                        valueObj->AssignParse(json);
                        overrideObj->Add(fieldName, valueObj);
                    }
                }
                s->Object->Add("GraphicsPipelineOverride", overrideObj);
            }
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
                        RCPtr<Texture> tex = RuntimeAssetManager::GetLoadedAssetByGuid<Texture>(guid);
                        m_sheet.SetTexture(name, tex);
                        break;
                    }
                    default:
                        break;
                    }
                }
            }

            // 读取 Queue
            if (auto queueObj = s->Object->At("Queue"))
            {
                uint32_t queueNum{};
                if (Enum::StaticTryParse(cltypeof<BoxingShaderPassRenderQueueType>(), queueObj->AsString(), &queueNum))
                {
                    m_queue = static_cast<ShaderPassRenderQueueType>(queueNum);
                }
            }

            // 读取 GraphicsPipelineOverride
            if (auto overrideObj = s->Object->At("GraphicsPipelineOverride"))
            {
                m_graphicsPipelineOverride = mksptr(new ShaderConfigGraphicsPipeline());
                m_graphicsPipelineOverrideFields = mksptr(new ObjectPropertyOverride());

                for (const auto& key : overrideObj->GetKeys())
                {
                    if (auto field = m_graphicsPipelineOverride->GetType()->GetFieldInfo(key))
                    {
                        auto valueObj = overrideObj->At(key);
                        auto value = ser::JsonSerializer::Deserialize(valueObj->ToString(), field->GetFieldType());
                        if (value)
                        {
                            field->SetValue(m_graphicsPipelineOverride.get(), value);
                            m_graphicsPipelineOverrideFields->Paths->push_back(key);
                        }
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

        if (!m_isDirtyParameter && !force)
            return;

        ApplyShaderDefaults();

        // 游戏线程：把 sheet 解析成渲染线程专用快照（纹理->句柄、常量纯值）。
        // 纹理的 CreateGPUResource 在此（游戏线程）触发，渲染线程不再触碰资产。
        RebuildRenderData();

        // 把新参数快照喂给渲染线程的 proxy；proxy 会更新自身快照并刷新所有就绪 binding。
        EnqueueProxyRenderData();

        m_isDirtyParameter = false;
    }

    void Material::EnqueueProxyShaderUpdate()
    {
        if (!m_renderProxy)
            return;
        auto* rt = Application::GetRenderThread();
        if (!rt)
            return;

        SPtr<ShaderConfig> config = m_shader ? m_shader->GetConfig() : nullptr;
        guid_t guid = m_shader ? m_shader->GetAssetGuid() : guid_t{};

        rt->EnqueueUpdate_AnyThread(
            [proxy = m_renderProxy, config = std::move(config), guid,
             features = m_activeFeatures, queue = m_queue,
             gpOverride = m_graphicsPipelineOverride, gpFields = m_graphicsPipelineOverrideFields]
            (gfx::GFXResourceManager*) mutable
            {
                proxy->UpdateShader(std::move(config), guid, std::move(features), queue,
                                       std::move(gpOverride), std::move(gpFields));
            });
    }

    void Material::EnqueueProxyStateUpdate()
    {
        if (!m_renderProxy)
            return;
        auto* rt = Application::GetRenderThread();
        if (!rt)
            return;

        rt->EnqueueUpdate_AnyThread(
            [proxy = m_renderProxy, queue = m_queue,
             gpOverride = m_graphicsPipelineOverride, gpFields = m_graphicsPipelineOverrideFields]
            (gfx::GFXResourceManager*) mutable
            {
                proxy->UpdateStateSnapshot(queue, std::move(gpOverride), std::move(gpFields));
            });
    }

    void Material::EnqueueProxyRenderData()
    {
        if (!m_renderProxy)
            return;
        auto* rt = Application::GetRenderThread();
        if (!rt)
            return;

        rt->EnqueueUpdate_AnyThread(
            [proxy = m_renderProxy, renderData = m_renderData](gfx::GFXResourceManager*) mutable
            {
                proxy->ApplyRenderData(std::move(renderData));
            });
    }

    void Material::PostEditChange(FieldInfo* info)
    {
        base::PostEditChange(info);
        auto& name = info->GetName();
        if (name == NAMEOF(m_shader))
        {
            SetShader(m_shader);
        }
        else if (name == NAMEOF(m_graphicsPipelineOverride) || name == NAMEOF(m_graphicsPipelineOverrideFields))
        {
            EnqueueProxyStateUpdate();
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
            {
                if (!m_sheet.HasProperty(prop->Name))
                {
                    // 格式 "r,g,b,a"
                    float r = 0, g = 0, b = 0, a = 1;
                    sscanf_s(prop->DefaultValue.c_str(), "%f,%f,%f,%f", &r, &g, &b, &a);
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

    void Material::RebuildRenderData()
    {
        // 游戏线程：解析纹理资产为 GPU 句柄、拷贝常量纯值。渲染线程消费 m_renderData。
        m_renderData = ShaderPropertySync::BuildRenderData(m_sheet);
    }

    void Material::SetShader(RCPtr<Shader> value)
    {
        m_shader = std::move(value);

        RuntimeObjectManager::RebuildMessageBox(this);

        // shader 变更时将 DefaultValue 写进 sheet（不覆盖已有值）
        ApplyShaderDefaults();

        if (m_createdGpuResource)
        {
            // 游戏线程：shader 默认值已写入 sheet，重建渲染线程快照。
            RebuildRenderData();
            // 把新 shader 快照喂给 proxy（清 bindings 触发重编译）+ 首份参数快照。
            EnqueueProxyShaderUpdate();
            EnqueueProxyRenderData();
        }

        OnShaderChanged.Invoke();
    }

    void Material::SetActiveFeatures(std::vector<std::string> features)
    {
        m_activeFeatures = std::move(features);
        if (m_createdGpuResource)
            EnqueueProxyShaderUpdate();
    }

    void Material::OnCollectAssetDependencies(array_list<guid_t>& deps)
    {
        base::OnCollectAssetDependencies(deps);
        deps.push_back(m_shader.GetGuid());

        // 收集参数中的纹理引用
        for (const auto& [name, value] : m_sheet.GetAllProperties())
        {
            if (value.GetType() == ShaderPropertyType::Texture2D ||
                value.GetType() == ShaderPropertyType::TextureCube)
            {
                if (auto tex = value.AsTexture2D())
                {
                    deps.push_back(tex->GetAssetGuid());
                }
            }
        }
    }

    void Material::SetOpaqueOverride()
    {
        if (!m_graphicsPipelineOverride)
            init_sptr_member(m_graphicsPipelineOverride);
        if (!m_graphicsPipelineOverrideFields)
            init_sptr_member(m_graphicsPipelineOverrideFields);

        m_graphicsPipelineOverride->Blend_Enabled = false;
        m_graphicsPipelineOverride->Blend_Src     = BlendFactor::One;
        m_graphicsPipelineOverride->Blend_Dst     = BlendFactor::Zero;
        m_graphicsPipelineOverride->ZWriteEnabled = true;

        m_graphicsPipelineOverrideFields->AddField("Blend_Enabled");
        m_graphicsPipelineOverrideFields->AddField("Blend_Src");
        m_graphicsPipelineOverrideFields->AddField("Blend_Dst");
        m_graphicsPipelineOverrideFields->AddField("ZWriteEnabled");

        EnqueueProxyStateUpdate();
    }

    void Material::SetTranslucentOverride()
    {
        if (!m_graphicsPipelineOverride)
            init_sptr_member(m_graphicsPipelineOverride);
        if (!m_graphicsPipelineOverrideFields)
            init_sptr_member(m_graphicsPipelineOverrideFields);

        m_graphicsPipelineOverride->Blend_Enabled = true;
        m_graphicsPipelineOverride->Blend_Src     = BlendFactor::SrcAlpha;
        m_graphicsPipelineOverride->Blend_Dst     = BlendFactor::OneMinusSrcAlpha;
        m_graphicsPipelineOverride->ZWriteEnabled = false;

        m_graphicsPipelineOverrideFields->AddField("Blend_Enabled");
        m_graphicsPipelineOverrideFields->AddField("Blend_Src");
        m_graphicsPipelineOverrideFields->AddField("Blend_Dst");
        m_graphicsPipelineOverrideFields->AddField("ZWriteEnabled");

        EnqueueProxyStateUpdate();
    }

    void Material::RestorePipelineDefaults()
    {
        SPtr<ShaderConfigGraphicsPipeline> shaderPipeline;
        if (m_shader && m_shader->GetConfig() && m_shader->GetConfig()->Passes && !m_shader->GetConfig()->Passes->empty())
        {
            if (auto pass0 = m_shader->GetConfig()->Passes->at(0))
                shaderPipeline = pass0->GraphicsPipeline;
        }
        if (!shaderPipeline)
            shaderPipeline = mksptr(new ShaderConfigGraphicsPipeline());

        if (!m_graphicsPipelineOverride)
            init_sptr_member(m_graphicsPipelineOverride);
        if (!m_graphicsPipelineOverrideFields)
            init_sptr_member(m_graphicsPipelineOverrideFields);

        auto type = cltypeof<ShaderConfigGraphicsPipeline>();
        for (auto fieldInfo : type->GetFieldInfos())
        {
            auto value = fieldInfo->GetValue(shaderPipeline.get());
            fieldInfo->SetValue(m_graphicsPipelineOverride.get(), value);
        }
        m_graphicsPipelineOverrideFields->Paths->clear();

        EnqueueProxyStateUpdate();
    }

    void Material::RebuildOverrideFields()
    {
        if (!m_graphicsPipelineOverride || !m_graphicsPipelineOverrideFields)
            return;

        SPtr<ShaderConfigGraphicsPipeline> shaderPipeline;
        if (m_shader && m_shader->GetConfig() && m_shader->GetConfig()->Passes && !m_shader->GetConfig()->Passes->empty())
        {
            if (auto pass0 = m_shader->GetConfig()->Passes->at(0))
                shaderPipeline = pass0->GraphicsPipeline;
        }
        if (!shaderPipeline)
            shaderPipeline = mksptr(new ShaderConfigGraphicsPipeline());

        m_graphicsPipelineOverrideFields->Paths->clear();

        auto type = cltypeof<ShaderConfigGraphicsPipeline>();
        for (auto fieldInfo : type->GetFieldInfos())
        {
            auto overrideValue = fieldInfo->GetValue(m_graphicsPipelineOverride.get());
            auto shaderValue   = fieldInfo->GetValue(shaderPipeline.get());
            if (overrideValue->ToString() != shaderValue->ToString())
            {
                m_graphicsPipelineOverrideFields->AddField(fieldInfo->GetName());
            }
        }

        EnqueueProxyStateUpdate();
    }

} // namespace pulsar