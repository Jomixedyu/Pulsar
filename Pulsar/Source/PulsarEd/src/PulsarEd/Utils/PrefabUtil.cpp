#include "Utils/PrefabUtil.h"

#include "EditorWorld.h"
#include "Editors/SceneEditor/SceneEditor.h"
#include "Pulsar/Scene.h"

namespace pulsared
{

    RCPtr<Scene> PrefabUtil::NewSceneFromPrefab(RCPtr<Prefab> prefab)
    {
        auto scene = Scene::StaticCreate(prefab->GetName());
        scene->CombineFrom(prefab);
        return scene;
    }

    void PrefabUtil::OpenPrefab(RCPtr<Prefab> prefab)
    {
        auto world = new EditorWorld(prefab->GetName());
        if (auto sceneEditor = SceneEditor::GetCurrent())
        {
            sceneEditor->PushPreviewWorld(std::unique_ptr<EditorWorld>{world});
        }
        world->AddDirectionalLight();
        world->GetCurrentCamera()->SetBackgroundColor({0.17f, 0.22f, 0.4f});
        // 直接把 Prefab 资产作为场景加载，编辑结果直接写回 Prefab
        world->LoadScene(prefab);
    }

    void PrefabUtil::ClosePrefabMode()
    {
        if (auto sceneEditor = SceneEditor::GetCurrent())
        {
            while(!sceneEditor->PreviewWorldStackEmpty())
            {
                sceneEditor->PopPreviewWorld();
            }
        }
    }

} // namespace pulsared