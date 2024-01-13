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
        ShaderEditorMenuContext(AssetObject_ref asset)
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
                    entry->Action = MenuAction::FromLambda([](const sptr<MenuContexts>& ctxs) {
                        const auto ctx = ctxs->FindContext<AssetEditorMenuContext>();
                        if (!ctx)
                            return;
                        if (const Shader_ref shader = ref_cast<Shader>(ctx->Asset))
                        {
                            ShaderCompiler::CompileShader(shader);
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
        Shader_ref shader = m_assetObject;

        m_previewMaterial = Material::StaticCreate("PreviewMaterial");
        m_previewMaterial->SetShader(m_assetObject);
        m_previewMaterial->CreateGPUResource();

        auto previewMesh = Node::StaticCreate("PreviewMesh");
        auto renderer = previewMesh->AddComponent<StaticMeshRendererComponent>();

        renderer->SetStaticMesh(GetAssetManager()->LoadAsset<StaticMesh>(BuiltinAsset::Shapes_Sphere));

        m_world->GetPersistentScene()->AddNode(previewMesh);

        if (m_previewMaterial->GetShader()->GetRenderingType() == ShaderPassRenderingType::PostProcessing)
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
        Shader_ref shader = m_assetObject;
        if (PImGui::PropertyGroup("Shader"))
        {
            PImGui::ObjectFieldProperties(
                BoxingObjectPtrBase::StaticType(),
                m_assetObject->GetType(),
                mkbox((ObjectPtrBase)m_assetObject).get(),
                m_assetObject.GetPtr());
        }

        if (PImGui::PropertyGroup("Compiled"))
        {
            string apis;
            for (auto api : shader->GetSupportedApi())
            {
                apis += gfx::to_string(api);
                apis += ";";
            }
            if (PImGui::BeginPropertyLine())
            {
                PImGui::PropertyLineText("Platforms", apis);
                PImGui::EndPropertyLine();
            }
        }
    }


    void ShaderEditorWindow::OnDrawImGui(float dt)
    {
        static MenuInit _MenuInit_;
        base::OnDrawImGui(dt);
    }

} // namespace pulsared