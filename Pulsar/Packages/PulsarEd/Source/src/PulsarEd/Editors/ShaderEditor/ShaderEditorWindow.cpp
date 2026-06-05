#include "Editors/ShaderEditor/ShaderEditorWindow.h"

#include "Pulsar/BuiltinAsset.h"
#include "Pulsar/Components/CameraComponent.h"
#include "Pulsar/Components/StaticMeshRendererComponent.h"
#include "Pulsar/Scene.h"

#include <PulsarEd/Menus/Menu.h>
#include <PulsarEd/Menus/MenuEntry.h>
#include <PulsarEd/Menus/MenuEntrySubMenu.h>
#include <PulsarEd/Menus/MenuRenderer.h>
#include <PulsarEd/PropertyControls/PropertyControl.h>

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
                        if (const RCPtr<Shader> shader = cast<Shader>(ctx->Asset))
                        {
                            // TODO: 迁移到新的 ShaderInstanceCache 编译流程
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

    }
    void ShaderEditorWindow::OnClose()
    {
        base::OnClose();
    }

    void ShaderEditorWindow::OnRefreshMenuContexts()
    {
        base::OnRefreshMenuContexts();
        m_menuBarCtxs->Contexts.push_back(mksptr(new ShaderEditorMenuContext{this->GetAssetObject()}));
    }


    void ShaderEditorWindow::OnDrawImGui(float dt)
    {
        static MenuInit _MenuInit_;
        base::OnDrawImGui(dt);

        RCPtr<Shader> shader = cast<Shader>(m_assetObject);
        if (PImGui::PropertyGroup("Shader"))
        {
            PImGui::ObjectFieldProperties(
                BoxingObjectPtrBase::StaticType(),
                m_assetObject->GetType(),
                mkbox(ObjectPtrBase(m_assetObject.GetHandle())).get(),
                m_assetObject.GetPtr());
        }
    }

} // namespace pulsared