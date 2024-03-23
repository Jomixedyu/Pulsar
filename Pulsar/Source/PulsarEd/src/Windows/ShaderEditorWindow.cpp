#include "Windows/ShaderEditorWindow.h"

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

namespace pulsared
{

    class ShaderEditorMenuContext : public AssetEditorMenuContext
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::ShaderEditorMenuContext, AssetEditorMenuContext);

    public:
        ShaderEditorMenuContext(RCPtr<AssetObject> asset)
            : base(asset)
        {
        }
    };
    namespace
    {
        struct MenuInit
        {
            static void _InitBuild(MenuEntrySubMenu_sp menu)
            {
                {
                    const auto entry = mksptr(new MenuEntrySeparate("Shader"));
                    menu->AddEntry(entry);
                }

                {
                    const auto entry = mksptr(new MenuEntryButton("Compile Shader"));
                    entry->Action = MenuAction::FromLambda([](const SPtr<MenuContexts>& ctxs) {
                        const auto ctx = ctxs->FindContext<AssetEditorMenuContext>();
                        if (!ctx)
                            return;
                        if (const RCPtr<Shader> shader = cref_cast<Shader>(ctx->Asset))
                        {
                            ShaderCompiler::CompileShader(shader.GetPtr());
                        }
                    });
                    menu->AddEntry(entry);
                }
            }
            MenuInit()
            {
                auto ae = MenuManager::GetOrAddMenu("AssetEditor");
                {
                    auto build = ae->FindOrNewMenuEntry("Build");
                    _InitBuild(build);
                }
            }
        };
    } // namespace

    void ShaderEditorWindow::OnOpen()
    {
        base::OnOpen();
        RCPtr<Shader> shader = cref_cast<Shader>(m_assetObject);

        m_previewMaterial = Material::StaticCreate("PreviewMaterial");
        m_previewMaterial->SetShader(shader);
        m_previewMaterial->CreateGPUResource();

        auto previewMesh = m_world->GetResidentScene()->NewNode("PreviewMesh");
        auto renderer = previewMesh->AddComponent<StaticMeshRendererComponent>();

        renderer->SetStaticMesh(GetAssetManager()->LoadAsset<StaticMesh>(BuiltinAsset::Shapes_Sphere));

        auto renderingType = m_previewMaterial->GetShader()->GetConfig()->RenderingType;
        if (renderingType == ShaderPassRenderingType::PostProcessing)
        {
            renderer->SetMaterial(0, GetAssetManager()->LoadAsset<Material>(BuiltinAsset::Material_Lambert));
            m_world->GetPreviewCamera()->m_postProcessMaterials->push_back(m_previewMaterial);
        }
        else
        {
            renderer->SetMaterial(0, m_previewMaterial);
        }

    }
    void ShaderEditorWindow::OnClose()
    {
        base::OnClose();
        DestroyObject(m_previewMaterial);
        m_previewMaterial.Reset();
    }

    void ShaderEditorWindow::OnRefreshMenuContexts()
    {
        base::OnRefreshMenuContexts();
        m_menuBarCtxs->Contexts.push_back(mksptr(new ShaderEditorMenuContext{this->GetAssetObject()}));
    }

    void ShaderEditorWindow::OnDrawAssetPropertiesUI(float dt)
    {
        base::OnDrawAssetPropertiesUI(dt);
        RCPtr<Shader> shader = cref_cast<Shader>(m_assetObject);
        if (PImGui::PropertyGroup("Shader"))
        {
            PImGui::ObjectFieldProperties(
                BoxingObjectPtrBase::StaticType(),
                m_assetObject->GetType(),
                mkbox(ObjectPtrBase(m_assetObject.GetHandle())).get(),
                m_assetObject.GetPtr());
        }

        if (PImGui::PropertyGroup("Shader Features"))
        {
            array_list<uint8_t> bools;
            array_list<size_t> changedIndex;
            bools.reserve(shader->GetConfig()->FeatureDeclare->size());
            for (auto& feature : *shader->GetConfig()->FeatureDeclare)
            {
                if (std::ranges::contains(shader->GetFeatureOptions(), feature))
                {
                    bools.push_back(true);
                }
                else
                {
                    bools.push_back(false);
                }
            }

            if (PImGui::BeginPropertyLines())
            {
                for (int i = 0; i < shader->GetConfig()->FeatureDeclare->size(); ++i)
                {
                    auto& item = shader->GetConfig()->FeatureDeclare->at(i);
                    auto boolObj = mkbox((bool)bools.at(i));
                    if (PImGui::PropertyLine(item, cltypeof<Boolean>(), boolObj.get()))
                    {
                        bools.at(i) = boolObj->get_unboxing_value();
                        changedIndex.push_back(i);
                    }
                }
                PImGui::EndPropertyLines();
            }

            for (size_t index : changedIndex)
            {
                auto& name = shader->GetConfig()->FeatureDeclare->at(index);
                if (bools.at(index))
                {
                    shader->GetFeatureOptions().push_back(name);
                }
                else
                {
                    std::erase(shader->GetFeatureOptions(), name);
                }
            }
            if (!changedIndex.empty())
            {
                AssetDatabase::MarkDirty(shader);
            }
        }

        if (PImGui::PropertyGroup("Compiled"))
        {
            string apis;
            for (auto api : shader->GetSupportedApi())
            {
                apis += gfx::to_string(api);
                apis += ";";
            }
            if (PImGui::BeginPropertyLines())
            {
                PImGui::PropertyLineText("Platforms", apis);
                PImGui::EndPropertyLines();
            }
        }
    }


    void ShaderEditorWindow::OnDrawImGui(float dt)
    {
        static MenuInit _MenuInit_;
        base::OnDrawImGui(dt);
    }

} // namespace pulsared