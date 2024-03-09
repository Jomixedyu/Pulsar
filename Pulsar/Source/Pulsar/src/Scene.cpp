#include "Components/ConstDataComponent.h"

#include <CoreLib.Serialization/DataSerializer.h>
#include <Pulsar/Node.h>
#include <Pulsar/Scene.h>
#include <ranges>

namespace pulsar
{
    static void _BeginNode(Scene_ref scene, Node_ref node)
    {
        if (!IsValid(node->GetRuntimeOwnerScene()))
        {
            node->BeginNode(scene);
        }
        // for (auto& child : *node->GetTransform()->GetChildren())
        // {
        //     _BeginNode(scene, child->GetAttachedNode());
        // }
    }
    void Scene::OnAddNode(Node_ref node)
    {
        if (m_runtimeWorld)
        {
            _BeginNode(self_ref(), node);
        }
    }

    void Scene::OnRemoveNode(Node_ref node)
    {
    }

    void Scene::BeginScene(World* world)
    {
        m_runtimeWorld = world;
        for (auto& node : *m_nodes)
        {
            node->BeginNode(self_ref());
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
        return;
        for (auto& node : *GetNodes())
        {
            if (IsValid(node) && node->GetIsActive())
            {
                node->OnTick(ticker);
            }
        }
    }

    Scene::Scene()
    {

    }

    ObjectPtr<Scene> Scene::StaticCreate(string_view name)
    {
        auto self = mksptr(new Scene);
        self->Construct();
        self->SetIndexName(name);
        self->SetObjectFlags(self->GetObjectFlags() | OF_Persistent | OF_Instantiable);

        return self;
    }

    void Scene::AddPrefab(RCPtr<Prefab> prefab)
    {
        auto prefabHandles = prefab->GetCollectionHandles().get();
        hash_map<guid_t, guid_t> map;

        array_list<Node_ref> addedNodes;
        addedNodes.reserve(prefab->GetCollectionHandles()->size());

        hash_map<Node_ref, Node_ref> newOldMapping;

        for (auto& node : *prefab->GetNodes())
        {
            auto newNode = BeginNewNode(node->GetIndexName());
            addedNodes.emplace_back(newNode);
            map[node.GetHandle()] = newNode.GetHandle();
            newOldMapping[newNode] = node;

            array_list<Component_ref> components;
            node->GetAllComponents(components);

            for (auto& component : components)
            {
                auto newComponent = newNode->AddComponent(component->GetType());
                map[component.GetHandle()] = newComponent.GetHandle();
            }
        }

        for (auto& addedNode : addedNodes)
        {
            auto finded = newOldMapping.find(addedNode);
            if (finded != newOldMapping.end())
            {
                auto oldNode = finded->second;
                for (size_t i = 0; i < oldNode->GetComponentCount(); ++i)
                {
                    auto oldComponent = oldNode->GetAllComponentArray()[i];

                    auto objser = ser::CreateVarient("json");
                    ComponentSerializer ser{objser, true, true};
                    oldComponent->Serialize(&ser);

                    auto newComponent = addedNode->GetAllComponentArray()[i];
                    ser.IsWrite = false;
                    ser.MovingTable = &map;
                    newComponent->Serialize(&ser);

                    if (auto transform = ref_cast<TransformComponent>(newComponent))
                    {
                        if (transform->GetParent())
                        {
                            UnregisterRootNode(addedNode);
                        }
                    }
                }
            }
        }
        for (auto& addedNode : addedNodes)
        {
            EndNewNode(addedNode);
        }
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
