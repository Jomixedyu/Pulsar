#include "Utils/PrefabUtil.h"

#include "EditorWorld.h"
#include "Pulsar/Scene.h"

namespace pulsared
{

    Scene_ref PrefabUtil::NewSceneFromPrefab(RCPtr<Prefab> prefab)
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
        world->AddGrid3d();
        world->AddDirectionalLight();
        world->GetPreviewCamera()->SetBackgroundColor({0.17, 0.22,0.4});

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