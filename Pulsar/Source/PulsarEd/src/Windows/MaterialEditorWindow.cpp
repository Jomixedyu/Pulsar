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
                mkbox(ObjectPtrBase(m_assetObject.Handle)).get(),
                m_assetObject.GetPtr());
        }

        RCPtr<Material> material = cref_cast<Material>(m_assetObject);
        if (material)
        {
            if (m_shader != material->GetShader())
            {
                OnShaderChanged(material->GetShader());
            }
            m_shader = material->GetShader();
        }

        if (PImGui::PropertyGroup("Parameters"))
        {
            if (PImGui::BeginPropertyLines())
            {
                for (auto& name : material->GetShader()->GetPropertyNames())
                {
                    auto& prop = *material->GetShader()->GetPropertyInfo(name);
                    const auto paramType = prop.Value.Type;
                    Object_sp obj;
                    Type* objType{};
                    switch (paramType)
                    {
                    case ShaderParameterType::IntScalar: {
                        obj = mkbox(material->GetIntScalar(name));
                        objType = obj->GetType();
                        break;
                    }
                    case ShaderParameterType::Scalar: {
                        obj = mkbox(material->GetScalar(name));
                        objType = obj->GetType();
                        break;
                    }
                    case ShaderParameterType::Vector: {
                        const auto vec = material->GetVector4(name);
                        obj = mkbox(Color4f{vec.x, vec.y, vec.z, vec.w});
                        objType = obj->GetType();
                        break;
                    }
                    case ShaderParameterType::Texture2D: {
                        auto tex = material->GetTexture(name);
                        objType = Texture2D::StaticType();
                        obj = mkbox(ObjectPtrBase(tex.GetHandle()));
                        break;
                    }
                    }

                    if (PImGui::PropertyLine(name.to_string(), objType, obj.get()))
                    {
                        AssetDatabase::MarkDirty(m_assetObject);
                        switch (paramType)
                        {
                        case ShaderParameterType::IntScalar:
                            material->SetIntScalar(name, UnboxUtil::Unbox<int>(obj));
                            break;
                        case ShaderParameterType::Scalar:
                            material->SetFloat(name, UnboxUtil::Unbox<float>(obj));
                            break;
                        case ShaderParameterType::Vector: {
                            auto color = UnboxUtil::Unbox<Color4f>(obj);
                            material->SetVector4(name, {color.r, color.g, color.b, color.a});
                            break;
                        }
                        case ShaderParameterType::Texture2D: {
                            auto objptr = UnboxUtil::Unbox<ObjectPtrBase>(obj);
                            RCPtr<Texture2D> tex = objptr.GetHandle();
                            material->SetTexture(name, tex);
                            break;
                        }
                        }
                        material->SubmitParameters();
                    }
                }
                PImGui::EndPropertyLines();
            }
        }
    }

    void MaterialEditorWindow::OnOpen()
    {
        base::OnOpen();
        RCPtr<Material> material = m_assetObject;
        material->CreateGPUResource();

        auto previewMesh =m_world->GetResidentScene()->NewNode("PreviewMesh");
        m_previewMeshRenderer = previewMesh->AddComponent<StaticMeshRendererComponent>();
        m_previewMeshRenderer->SetStaticMesh(GetAssetManager()->LoadAsset<StaticMesh>(BuiltinAsset::Shapes_Sphere));

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
        if (newShader == nullptr)
        {
            m_world->GetCurrentCamera()->ClearPostProcess();
            m_previewMeshRenderer->SetMaterial(0, m_assetObject);
        }
        else if (newShader->GetConfig()->RenderingType == ShaderPassRenderingType::PostProcessing)
        {
            if (m_world->GetCurrentCamera()->GetPostProcessCount() == 0)
            {
                m_world->GetCurrentCamera()->AddPostProcess(m_assetObject);
            }
            else
            {
                m_world->GetCurrentCamera()->SetPostProcess(0, m_assetObject);
            }
            m_previewMeshRenderer->SetMaterial(0, GetAssetManager()->LoadAsset<Material>(BuiltinAsset::Material_Lambert));
        }
        else
        {
            m_world->GetCurrentCamera()->ClearPostProcess();
            m_previewMeshRenderer->SetMaterial(0, m_assetObject);
        }

    }

} // namespace pulsared