#include "Pulsar/Assets/Texture2D.h"
#include "Pulsar/BuiltinAsset.h"
#include "Pulsar/Components/CameraComponent.h"
#include "Pulsar/Components/StaticMeshRendererComponent.h"
#include "Pulsar/Scene.h"

#include <PulsarEd/Menus/Menu.h>
#include <PulsarEd/Menus/MenuEntry.h>
#include <PulsarEd/Menus/MenuEntrySubMenu.h>
#include <PulsarEd/Menus/MenuRenderer.h>
#include <PulsarEd/PropertyControls/PropertyControl.h>
#include <PulsarEd/Editors/MaterialEditor//MaterialEditorWindow.h>

namespace pulsared
{

    void MaterialEditorWindow::OnDrawAssetPropertiesUI(float dt)
    {
        base::OnDrawAssetPropertiesUI(dt);

        if (PImGui::PropertyGroup("Material"))
        {
            PImGui::ObjectFieldProperties(
                BoxingRCPtrBase::StaticType(),
                m_assetObject->GetType(),
                mkbox( (RCPtrBase)m_assetObject ).get(),
                m_assetObject.GetPtr());
        }

        RCPtr<Material> material = cast<Material>(m_assetObject);
        if (material)
        {
            if (m_shader != material->GetShader())
            {
                OnShaderChanged(material->GetShader());
                AssetDatabase::MarkDirty(m_assetObject);
            }
            m_shader = material->GetShader();
        }

        if (PImGui::PropertyGroup("Parameters"))
        {
            if (PImGui::BeginPropertyLines())
            {
                auto shader = material->GetShader();
                auto shaderConfig = shader ? shader->GetConfig() : nullptr;
                if (shaderConfig && shaderConfig->Properties)
                {
                    for (const auto& prop : *shaderConfig->Properties)
                    {
                        index_string name{prop->Name};
                        const auto paramType = prop->Type;
                        Object_sp obj;
                        Type* objType{};
                        switch (paramType)
                        {
                        case ShaderPropertyType::Int: {
                            obj = mkbox(material->GetIntScalar(name));
                            objType = obj->GetType();
                            break;
                        }
                        case ShaderPropertyType::Float: {
                            obj = mkbox(material->GetScalar(name));
                            objType = obj->GetType();
                            break;
                        }
                        case ShaderPropertyType::Float4:
                        case ShaderPropertyType::Color: {
                            const auto vec = material->GetVector4(name);
                            obj = mkbox(Color4f{vec.x, vec.y, vec.z, vec.w});
                            objType = obj->GetType();
                            break;
                        }
                        case ShaderPropertyType::Texture2D: {
                            auto tex = material->GetTexture(name);
                            objType = Texture::StaticType();
                            obj = mkbox((RCPtrBase&)tex);
                            break;
                        }
                        default:
                            break;
                        }

                        const auto& label = !prop->Label.empty() ? prop->Label : prop->Name;
                        if (obj && PImGui::PropertyLine(label, objType, obj.get()))
                        {
                            AssetDatabase::MarkDirty(m_assetObject);
                            switch (paramType)
                            {
                            case ShaderPropertyType::Int:
                                material->SetIntScalar(name, UnboxUtil::Unbox<int>(obj));
                                break;
                            case ShaderPropertyType::Float:
                                material->SetFloat(name, UnboxUtil::Unbox<float>(obj));
                                break;
                            case ShaderPropertyType::Float4:
                            case ShaderPropertyType::Color: {
                                auto color = UnboxUtil::Unbox<Color4f>(obj);
                                material->SetVector4(name, {color.r, color.g, color.b, color.a});
                                break;
                            }
                            case ShaderPropertyType::Texture2D: {
                                auto objptr = UnboxUtil::Unbox<RCPtrBase>(obj);
                                RCPtr<Texture> tex = cast<Texture>(objptr);
                                material->SetTexture(name, tex);
                                break;
                            }
                            default:
                                break;
                            }
                            material->SubmitParameters();
                        }
                    }
                }
                PImGui::EndPropertyLines();
            }
        }
    }

    void MaterialEditorWindow::OnOpen()
    {
        base::OnOpen();
        RCPtr<Material> material = cast<Material>(m_assetObject);
        material->CreateGPUResource();

        auto previewMesh =m_world->GetResidentScene()->NewNode("PreviewMesh");
        m_previewMeshRenderer = previewMesh->AddComponent<StaticMeshRendererComponent>();
        m_previewMeshRenderer->SetStaticMesh(AssetManager::Get()->LoadAsset<StaticMesh>(BuiltinAsset::Shapes_Sphere));

    }
    void MaterialEditorWindow::OnClose()
    {
        base::OnClose();
    }

    void MaterialEditorWindow::OnRefreshMenuContexts()
    {
        base::OnRefreshMenuContexts();
        // m_menuBarCtxs->Contexts.push_back();
    }
    void MaterialEditorWindow::OnShaderChanged(const RCPtr<Shader>& newShader)
    {
        RCPtr<Material> material = cast<Material>(m_assetObject);
        if (newShader == nullptr)
        {
            m_world->GetCurrentCamera()->ClearPostProcess();
            m_previewMeshRenderer->SetMaterial(0, material);
        }
        else
        {
            // TODO: 通过 ShaderConfigPass::Queue 判断 PostProcess
            m_world->GetCurrentCamera()->ClearPostProcess();
            m_previewMeshRenderer->SetMaterial(0, material);
        }

    }

} // namespace pulsared