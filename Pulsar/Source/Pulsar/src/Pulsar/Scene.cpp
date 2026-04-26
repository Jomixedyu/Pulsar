#include "Components/ConstDataComponent.h"

#include <CoreLib.Serialization/DataSerializer.h>
#include <Pulsar/Node.h>
#include <Pulsar/Scene.h>
#include <ranges>

namespace pulsar
{
    void Scene::OnAddNode(ObjectPtr<Node> node)
    {
        base::OnAddNode(node);
    }

    void Scene::OnRemoveNode(ObjectPtr<Node> node)
    {
        base::OnRemoveNode(node);
    }

    void Scene::Serialize(AssetSerializer* s)
    {
        base::Serialize(s);
    }

    void Scene::OnInstantiateAsset(AssetObject* obj)
    {
        base::OnInstantiateAsset(obj);
    }

    Scene::Scene()
    {

    }

    RCPtr<Scene> Scene::StaticCreate(string_view name)
    {
        auto self = NewAssetObject<Scene>();
        self->SetIndexName(name);
        self->SetObjectFlags(self->GetObjectFlags() | OF_Instantiable);

        return self;
    }

    void Scene::OnDestroy()
    {
        base::OnDestroy();
    }
} // namespace pulsar
