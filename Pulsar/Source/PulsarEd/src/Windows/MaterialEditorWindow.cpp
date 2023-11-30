#include <PulsarEd/Menus/Menu.h>
#include <PulsarEd/Menus/MenuEntry.h>
#include <PulsarEd/Menus/MenuEntrySubMenu.h>
#include <PulsarEd/Menus/MenuRenderer.h>
#include <PulsarEd/PropertyControls/PropertyControl.h>
#include <PulsarEd/Shaders/EditorShader.h>
#include <PulsarEd/UIControls/ViewportFrame.h>
#include <PulsarEd/Windows/MaterialEditorWindow.h>

namespace pulsared
{
    class AssetEditorMenuContext : public MenuContextBase
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::AssetEditorMenuContext, MenuContextBase);

    public:
        AssetEditorMenuContext(AssetObject_ref asset)
            : Asset(asset)
        {
        }
        AssetObject_ref Asset;
    };

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
            static void _InitFile(MenuEntrySubMenu_sp menu)
            {
                {
                    auto entry = mksptr(new MenuEntryButton("OpenAsset", "Open Asset"));

                    menu->AddEntry(entry);
                }

                {
                    auto entry = mksptr(new MenuEntryButton("Save", "Save"));
                    entry->Action = MenuAction::FromLambda([](MenuContexts_sp ctxs) { 
                        {
                            auto ctx = ctxs->FindContext<AssetEditorMenuContext>();
                            if (!ctx) return;
                            AssetDatabase::Save(ctx->Asset);
                        } });
                    menu->AddEntry(entry);
                }
            }
            static void _InitBuild(MenuEntrySubMenu_sp menu)
            {
                {
                    auto entry = mksptr(new MenuEntrySeparate("Shader"));

                    menu->AddEntry(entry);
                }

                {
                    auto entry = mksptr(new MenuEntryButton("Compile Shader"));
                    entry->Action = MenuAction::FromLambda([](sptr<MenuContexts> ctxs) {
                        {
                            auto ctx = ctxs->FindContext<AssetEditorMenuContext>();
                            if (!ctx) return;
                            if (Shader_ref shader = ref_cast<Shader>(ctx->Asset))
                            {
                                ShaderCompiler::CompileShader(shader, { gfx::GFXApi::Vulkan }, {}, {});
                            }
                        } });
                    menu->AddEntry(entry);
                }
            }
            MenuInit()
            {
                auto ae = MenuManager::GetOrAddMenu("AssetEditor");
                {
                    auto file = ae->FindOrNewMenuEntry("File");
                    _InitFile(file);
                }
                {
                    auto build = ae->FindOrNewMenuEntry("Build");
                    _InitBuild(build);
                }
            }
        };
    } // namespace

    void ShaderEditorWindow::OnOpen()
    {
    }

    void ShaderEditorWindow::OnDrawImGui()
    {
        static MenuInit _MenuInit_;

        if (ImGui::BeginMenuBar())
        {
            auto ctxs = mksptr(new MenuContexts);
            ctxs->Contexts.push_back(mksptr(new ShaderEditorMenuContext{this->GetAssetObject()}));
            MenuRenderer::RenderMenu(MenuManager::GetMenu("AssetEditor").get(), ctxs);
            ImGui::EndMenuBar();
        }

        if (!m_assetObject)
        {
            ImGui::Text("no asset");
            return;
        }

        ImGui::Columns(2);

        if (ImGui::BeginChild("#Preview"))
        {
            auto descSet = AssetDatabase::IconPool->GetDescriptorSet({cltypeof<AssetObject>()->GetName()});
            auto size = PImGui::GetContentSize();
            ImGui::Image((void*)descSet.lock()->GetId(), ImVec2(size.x, size.y));

            ImGui::EndChild();
        }

        ImGui::NextColumn();

        if (ImGui::BeginChild("Properties"))
        {
            if (PImGui::PropertyGroup("Asset Info"))
            {
                if (PImGui::BeginPropertyItem("Asset Path"))
                {
                    ImGui::Text(AssetDatabase::GetPathByAsset(m_assetObject).c_str());
                    PImGui::EndPropertyItem();
                }
                if (PImGui::BeginPropertyItem("Object Id"))
                {
                    ImGui::Text(m_assetObject.handle.to_string().c_str());
                    PImGui::EndPropertyItem();
                }
            }
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
                if (PImGui::BeginPropertyItem("Platforms"))
                {
                    for (auto api : shader->GetSupportedApi())
                    {
                        ImGui::Text(gfx::to_string(api));
                    }
                    PImGui::EndPropertyItem();
                }
                // PImGui::PropertyLine("Asset Path", m_assetObject->GetType(), m_assetObject.GetPtr());
            }

            ImGui::EndChild();
        }

        ImGui::Columns(1);
    }
} // namespace pulsared