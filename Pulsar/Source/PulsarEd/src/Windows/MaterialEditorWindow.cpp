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
#include <PulsarEd/Shaders/EditorShader.h>
#include <PulsarEd/Windows/MaterialEditorWindow.h>

namespace pulsared
{

    void MaterialEditorWindow::OnDrawAssetPropertiesUI(float dt)
    {
        base::OnDrawAssetPropertiesUI(dt);

        if (PImGui::PropertyGroup("Material"))
        {
            PImGui::ObjectFieldProperties(
                BoxingObjectPtrBase::StaticType(),
                m_assetObject->GetType(),
                mkbox((ObjectPtrBase)m_assetObject).get(),
                m_assetObject.GetPtr());
        }

        Material_ref material = m_assetObject;
        if (m_shader != material->GetShader())
        {
            // m_world->GetPersistentScene()
            //     ->FindNodeByName("PreviewMesh")
            //     ->GetComponent<StaticMeshRendererComponent>()
            //     ->SetMaterial()
        }
        if (PImGui::PropertyGroup("Parameters"))
        {
            if (PImGui::BeginPropertyLine())
            {
                for (auto& prop : material->GetShaderPropertyInfo())
                {
                    const auto paramType = prop.second.Value.Type;
                    Object_sp obj;
                    Type* objType{};
                    switch (paramType)
                    {
                    case ShaderParameterType::Scalar:
                        obj = mkbox(material->GetFloat(prop.first));
                        objType = obj->GetType();
                        break;
                    case ShaderParameterType::Vector: {
                        const auto vec = material->GetVector4(prop.first);
                        obj = mkbox(Color4f{vec.x, vec.y, vec.z, vec.w});
                        objType = obj->GetType();
                        break;
                    }
                    case ShaderParameterType::Texture2D: {
                        auto tex = material->GetTexture(prop.first);
                        objType = Texture2D::StaticType();
                        obj = mkbox((ObjectPtrBase)tex);
                        break;
                    }
                    }

                    if (PImGui::PropertyLine(prop.first.to_string(), objType, obj.get()))
                    {
                        AssetDatabase::MarkDirty(m_assetObject);
                        switch (paramType)
                        {
                        case ShaderParameterType::IntScalar:
                            material->SetIntScalar(prop.first, UnboxUtil::Unbox<int>(obj));
                            break;
                        case ShaderParameterType::Scalar:
                            material->SetFloat(prop.first, UnboxUtil::Unbox<float>(obj));
                            break;
                        case ShaderParameterType::Vector: {
                            auto color = UnboxUtil::Unbox<Color4f>(obj);
                            material->SetVector4(prop.first, {color.r, color.g, color.b, color.a});
                            break;
                        }
                        case ShaderParameterType::Texture2D:

                            break;
                        }
                        material->SubmitParameters();
                    }
                }
                PImGui::EndPropertyLine();
            }
        }
    }
    void MaterialEditorWindow::OnOpen()
    {
        base::OnOpen();
        Material_ref material = m_assetObject;
        material->CreateGPUResource();

        auto previewMesh = Node::StaticCreate("PreviewMesh");
        auto renderer = previewMesh->AddComponent<StaticMeshRendererComponent>();
        renderer->SetStaticMesh(GetAssetManager()->LoadAsset<StaticMesh>(BuiltinAsset::Shapes_Sphere));

        if (material->GetRenderingType() == ShaderPassRenderingType::PostProcessing)
        {
            m_world->GetPreviewCamera()->m_postProcessMaterials->push_back(material);
            renderer->SetMaterial(0, GetAssetManager()->LoadAsset<Material>(BuiltinAsset::Material_Lambert));
        }
        else
        {
            renderer->SetMaterial(0, m_assetObject);
        }

        m_world->GetPersistentScene()->AddNode(previewMesh);
        m_shader = material->GetShader();
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
} // namespace pulsared