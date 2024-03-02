#include "Windows/StaticMeshEditorWindow.h"
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

namespace pulsared
{

    void StaticMeshEditorWindow::OnDrawAssetPropertiesUI(float dt)
    {
        base::OnDrawAssetPropertiesUI(dt);

    }
    void StaticMeshEditorWindow::OnOpen()
    {
        base::OnOpen();

        m_staticmesh = ref_cast<StaticMesh>(m_assetObject);
        m_staticmesh->Incref();
        m_staticmesh->CreateGPUResource();

        auto previewMesh = m_world->GetResidentScene()->NewNode("PreviewMesh");
        auto renderer = previewMesh->AddComponent<StaticMeshRendererComponent>();
        renderer->SetStaticMesh(m_staticmesh);

        for (int i = 0; i < renderer->GetMaterialCount(); ++i)
        {
            renderer->SetMaterial(i, GetAssetManager()->LoadAsset<Material>(BuiltinAsset::Material_Lambert));
        }
    }
    void StaticMeshEditorWindow::OnClose()
    {
        base::OnClose();
        if (m_staticmesh)
        {
            m_staticmesh->Decref();
        }
    }

    void StaticMeshEditorWindow::OnRefreshMenuContexts()
    {
        base::OnRefreshMenuContexts();
        // m_menuBarCtxs->Contexts.push_back();
    }
} // namespace pulsared