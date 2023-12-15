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

    void MaterialEditorWindow::OnOpen()
    {
        base::OnOpen();
        auto previewMesh = Node::StaticCreate("PreviewMesh");
        auto renderer = previewMesh->AddComponent<StaticMeshRendererComponent>();

        renderer->SetStaticMesh(GetAssetManager()->LoadAsset<StaticMesh>("Engine/Shapes/Sphere"));
        renderer->SetMaterial(0, m_assetObject);
        m_world->GetPersistentScene()->AddNode(previewMesh);
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