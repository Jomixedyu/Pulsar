#include "Utils/PrefabUtil.h"

#include "EditorWorld.h"
#include "Pulsar/Scene.h"

namespace pulsared
{

    RCPtr<Scene> PrefabUtil::NewSceneFromPrefab(RCPtr<Prefab> prefab)
    {
        auto scene = Scene::StaticCreate(prefab->GetName());
        //scene->CopyFrom(prefab);
        scene->AddPrefab(prefab);
        return scene;
    }

    void PrefabUtil::OpenPrefab(RCPtr<Prefab> prefab)
    {
        auto world = new EditorWorld(prefab->GetName());
        EditorWorld::PushPreviewWorld(std::unique_ptr<EditorWorld>{world});
        world->AddDirectionalLight();
        world->GetCurrentCamera()->SetBackgroundColor({0.17f, 0.22f,0.4f});

        auto scene = NewSceneFromPrefab(prefab);
        world->LoadScene(scene);

    }

    void PrefabUtil::ClosePrefabMode()
    {
        while(!EditorWorld::PreviewWorldStackEmpty())
        {
            EditorWorld::PopPreviewWorld();
        }
    }

} // namespace pulsared