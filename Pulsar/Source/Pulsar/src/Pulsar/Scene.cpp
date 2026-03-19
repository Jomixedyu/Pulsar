#include "Components/ConstDataComponent.h"

#include <CoreLib.Serialization/DataSerializer.h>
#include <Pulsar/Node.h>
#include <Pulsar/Scene.h>
#include <ranges>

namespace pulsar
{
    static void _BeginNode(Scene_ref scene, ObjectPtr<Node> node)
    {
        if (!node->GetRuntimeOwnerScene())
        {
            node->BeginNode(scene);
        }
        // for (auto& child : *node->GetTransform()->GetChildren())
        // {
        //     _BeginNode(scene, child->GetAttachedNode());
        // }
    }
    void Scene::OnAddNode(ObjectPtr<Node> node)
    {
        if (m_runtimeWorld)
        {
            _BeginNode(self_ptr(), node);
        }
    }

    void Scene::OnRemoveNode(ObjectPtr<Node> node)
    {
        if (m_runtimeWorld)
        {
            node->EndNode();
        }
    }

    void Scene::BeginScene(World* world)
    {
        m_runtimeWorld = world;
        for (auto& node : *m_nodes)
        {
            node->BeginNode(self_ptr());
        }
        for (auto& node : *m_rootNodes)
        {
            node->GetTransform()->MakeTransformChanged();
        }
    }
    void Scene::EndScene()
    {
        for (auto& node : *m_nodes)
        {
            node->EndNode();
        }

        m_runtimeWorld = nullptr;
    }

    void Scene::Tick(Ticker ticker)
    {
        for (auto& node : *GetNodes())
        {
            if (node && node->GetIsActive())
            {
                node->OnTick(ticker);
            }
        }
    }
    void Scene::BeginPlay()
    {
        for (auto& node : *GetNodes())
        {
            if (node && node->GetIsActive())
            {
                node->BeginPlay();
            }
        }
    }
    void Scene::EndPlay()
    {
        for (auto& node : *GetNodes())
        {
            if (node && node->GetIsActive())
            {
                node->EndPlay();
            }
        }
    }
    void Scene::Serialize(AssetSerializer* s)
    {
        base::Serialize(s);
    }
    void Scene::OnInstantiateAsset(AssetObject* obj)
    {
        NodeCollection::OnInstantiateAsset(obj);
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

    void Scene::AddPrefab(RCPtr<Prefab> prefab)
    {
        //TODO: replace to combineFrom
//
//        auto prefabHandles = prefab->GetCollectionHandles().get();
//        //hash_map<guid_t, guid_t> map;
//
//        array_list<ObjectPtr<Node>> addedNodes;
//        addedNodes.reserve(prefab->GetCollectionHandles()->size());
//
//        hash_map<ObjectPtr<Node>, ObjectPtr<Node>> newOldMapping;
//
//        for (auto& nodePrefab : *prefab->GetNodes())
//        {
//            auto newNode = BeginNewNode(nodePrefab->GetIndexName());
//            newNode->m_sourceSceneObjectGuid = nodePrefab->GetSceneObjectGuid();
//            addedNodes.emplace_back(newNode);
//            map[nodePrefab.GetHandle()] = newNode.GetHandle();
//            newOldMapping[newNode] = nodePrefab;
//
//            array_list<Component_ref> components;
//            nodePrefab->GetAllComponents(components);
//
//            for (auto& component : components)
//            {
//                auto newComponent = newNode->AddComponent(component->GetType());
//                map[component.GetHandle()] = newComponent.GetHandle();
//            }
//        }
//
//        for (auto& addedNode : addedNodes)
//        {
//            auto found = newOldMapping.find(addedNode);
//            if (found != newOldMapping.end())
//            {
//                auto oldNode = found->second;
//                for (size_t i = 0; i < oldNode->GetComponentCount(); ++i)
//                {
//                    auto oldComponent = oldNode->GetAllComponentArray()[i];
//
//                    auto objser = ser::CreateVarient("json");
//                    ComponentSerializer ser{objser, true, true};
//                    oldComponent->Serialize(&ser);
//
//                    auto newComponent = addedNode->GetAllComponentArray()[i];
//                    ser.IsWrite = false;
//                    ser.MovingTable = &map;
//                    newComponent->Serialize(&ser);
//
//                    if (auto transform = cast<TransformComponent>(newComponent))
//                    {
//                        if (transform->GetParent())
//                        {
//                            UnregisterRootNode(addedNode);
//                        }
//                    }
//                }
//            }
//        }
//        for (auto& addedNode : addedNodes)
//        {
//            EndNewNode(addedNode);
//        }
    }

    void Scene::OnDestroy()
    {
        base::OnDestroy();

        for (auto& node : *m_nodes)
        {
            DestroyObject(node);
        }
    }
} // namespace pulsar
