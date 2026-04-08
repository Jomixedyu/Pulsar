#include "Assets/NodeCollection.h"
#include <Pulsar/Node.h>
#include <Pulsar/World.h>

#include <utility>
#include <sstream>
#include <CoreLib.Serialization/JsonSerializer.h>

namespace pulsar
{
    void NodeCollection::Serialize(AssetSerializer* s)
    {
        base::Serialize(s);

        if (s->IsWrite)
        {
            auto nodes = s->Object->New(ser::VarientType::Array);
            for (auto& node : *m_nodes)
            {
                if (node->IsTransientObject())
                {
                    continue;
                }

                auto nodeObj = s->Object->New(ser::VarientType::Object);
                nodeObj->Add("Id", node->GetSceneObjectGuid().to_string());

                SceneObjectSerializer nodeSerializer{nodeObj, s->IsWrite, s->HasEditorData, this};
                node->BeginSerialize(&nodeSerializer);
                nodes->Push(nodeObj);
            }
            s->Object->Add("Nodes", nodes);

            auto rootNodes = s->Object->New(ser::VarientType::Array);
            for (auto node : *m_rootNodes)
            {
                rootNodes->Push(node->GetSceneObjectGuid().to_string());
            }
            s->Object->Add("RootNodes", rootNodes);
        }
        else
        {
            struct NodeInfo
            {
                ObjectPtr<Node> Node;
                jxcorlib::ser::VarientRef NodeSerializer;
            };
            array_list<NodeInfo> nodes;

            auto nodesArr = s->Object->At("Nodes");
            for (int i = 0; i < nodesArr->GetCount(); ++i)
            {
                auto serNode = nodesArr->At(i);
                auto nodeId = guid_t::parse(serNode->At("Id")->AsString());
                auto newNode = ConstructNode("Node", nodeId);
                AddSceneObjectToFinder(newNode);
                nodes.push_back({ newNode, serNode });
            }

            for (auto& nodeInfo : nodes)
            {
                SceneObjectSerializer nodeSerializer{nodeInfo.NodeSerializer, s->IsWrite, s->HasEditorData, this};
                nodeInfo.Node->BeginSerialize(&nodeSerializer);
            }
            for (auto& nodeInfo : nodes)
            {
                SceneObjectSerializer nodeSerializer{nodeInfo.NodeSerializer, s->IsWrite, s->HasEditorData, this};
                nodeInfo.Node->EndSerialize(&nodeSerializer);
            }

            auto rootNodeArr = s->Object->At("RootNodes");
            for (int i = 0; i < rootNodeArr->GetCount(); ++i)
            {
                auto node = rootNodeArr->At(i);
                auto sceneGuid = guid_t::parse(node->AsString());
                RegisterRootNode(cast<Node>(FindSceneObject(sceneGuid)));
            }
        }
    }

    void NodeCollection::OnDestroy()
    {
        base::OnDestroy();
        const auto count = m_rootNodes->size();
        for (int i = count - 1; i >= 0; --i)
        {
            RemoveNode(m_rootNodes->at(i));
        }
    }

    ObjectPtr<Node> NodeCollection::FindNodeByName(index_string name) const
    {
        for (const auto& node : *m_rootNodes)
        {
            if (node->GetIndexName() == name)
            {
                return node;
            }
        }
        return {};
    }

    ObjectPtr<Node> NodeCollection::FindNodeByPath(string_view name) const
    {
        assert(false);
        return ObjectPtr<Node>();
    }

    void NodeCollection::OnInstantiateAsset(AssetObject* obj)
    {
        base::OnInstantiateAsset(obj);
    }

    ObjectPtr<Node> NodeCollection::NewNode(index_string name, const ObjectPtr<Node>& parent, ObjectFlags flags)
    {
        auto node = BeginNewNode(name, parent, flags);
        EndNewNode(node);
        return node;
    }

    void NodeCollection::RemoveNode(ObjectPtr<Node> node)
    {
        const auto transform = node->GetTransform();
        const auto count = (int)transform->GetChildCount();
        for (int i = count - 1; i >= 0; --i)
        {
            const auto child = transform->GetChildren()->at(i)->GetNode();
            RemoveNode(child);
            transform->GetChildren()->RemoveAt(i);
        }

        OnRemoveNode(node);

        std::erase(*m_nodes, node);
        for (size_t i = 0; i < m_rootNodes->size(); ++i)
        {
            if (m_rootNodes->at(i).GetHandle() == node.GetHandle())
            {
                m_rootNodes->RemoveAt((int)i);
                break;
            }
        }

        DestroyObject(node, true);
        std::erase(*m_collectionHandles, node.GetHandle());
    }

    void NodeCollection::RegisterRootNode(const ObjectPtr<Node>& node)
    {
        m_rootNodes->push_back(node);
    }

    void NodeCollection::UnregisterRootNode(const ObjectPtr<Node>& node)
    {
        std::erase(*m_rootNodes, node);
    }

    void NodeCollection::CopyFrom(ObjectPtr<NodeCollection> nc)
    {
    }

    NodeCollection::NodeCollection()
    {
        init_sptr_member(m_rootNodes);
        init_sptr_member(m_nodes);
        init_sptr_member(m_collectionHandles);
    }

    ObjectPtr<Node> NodeCollection::BeginNewNode(index_string name, const ObjectPtr<Node>& parent, ObjectFlags flags)
    {
        auto node = ConstructNode(name, {}, flags);
        if (parent)
        {
            node->SetParent(parent);
        }
        else
        {
            m_rootNodes->push_back(node);
        }
        m_collectionHandles->push_back(node.GetHandle());
        return node;
    }

    void NodeCollection::EndNewNode(ObjectPtr<Node> node)
    {
        OnAddNode(node);
    }

    ObjectPtr<Node> NodeCollection::ConstructNode(index_string name, guid_t guid, ObjectFlags flags)
    {
        auto newNode = mksptr(new Node);
        newNode->SetIndexName(name);
        newNode->SetObjectFlags(newNode->GetObjectFlags() | flags | OF_LifecycleManaged);
        newNode->SceneObjectConstruct(guid);

        auto node = cast<Node>(ObjectPtrBase{newNode->GetObjectHandle()});
        m_nodes->push_back(node);
        return node;
    }

    ObjectPtr<SceneObject> NodeCollection::FindSceneObject(guid_t sceneObjId) const
    {
        auto it = m_guidToNode.find(sceneObjId);
        if (it != m_guidToNode.end())
        {
            return it->second;
        }
        return {};
    }

    void NodeCollection::AddSceneObjectToFinder(const ObjectPtr<SceneObject>& obj)
    {
        m_guidToNode.insert({obj->GetSceneObjectGuid(), obj});
    }

    void NodeCollection::CombineFrom(RCPtr<NodeCollection> collection)
    {
        // 1. 把源 NodeCollection 序列化为 JSON + 二进制流
        const ser::VarientRef writeJsonRoot = ser::CreateVarient("json");
        std::stringstream writeBinary(std::ios::in | std::ios::out | std::ios::binary);
        {
            AssetSerializer writeS{writeJsonRoot, writeBinary, true, false};
            collection->Serialize(&writeS);
        }

        // 2. 构建 oldGuid -> newGuid 映射，做字符串级替换
        hash_map<string, string> guidRemap;
        auto nodesArr = writeJsonRoot->At("Nodes");
        if (nodesArr)
        {
            for (int i = 0; i < nodesArr->GetCount(); ++i)
            {
                auto nodeObj = nodesArr->At(i);
                if (auto idObj = nodeObj->At("Id"))
                {
                    string oldGuid = idObj->AsString();
                    string newGuid = guid_t::create_new().to_string();
                    guidRemap[oldGuid] = newGuid;
                }
            }
        }

        // JSON 转字符串后做字符串级 GUID 全量替换
        string jsonStr = writeJsonRoot->ToString();
        for (auto& [oldG, newG] : guidRemap)
        {
            size_t pos = 0;
            while ((pos = jsonStr.find(oldG, pos)) != string::npos)
            {
                jsonStr.replace(pos, oldG.size(), newG);
                pos += newG.size();
            }
        }

        // 3. 把替换后的 JSON 反序列化进 this
        const ser::VarientRef readJsonRoot = ser::CreateVarient("json");
        readJsonRoot->AssignParse(jsonStr);
        writeBinary.seekg(0);
        AssetSerializer readS{readJsonRoot, writeBinary, false, false};
        Serialize(&readS);

        // 4. 记录 m_sourceSceneObjectGuid（Prefab 实例追踪来源）
        for (auto& [oldGuidStr, newGuidStr] : guidRemap)
        {
            auto newGuid = guid_t::parse(newGuidStr);
            auto oldGuid = guid_t::parse(oldGuidStr);
            if (auto obj = FindSceneObject(newGuid))
                obj->m_sourceSceneObjectGuid = oldGuid;
        }
    }

    void NodeCollection::OnCollectAssetDependencies(array_list<guid_t>& deps)
    {
        for (auto& node : *m_nodes)
        {
            node->GetDependenciesAsset(deps);
        }
    }

    void NodeCollection::BeginScene(World* world)
    {
        m_runtimeWorld = world;
        for (auto& node : *m_nodes)
        {
            node->BeginNode(this);
        }
        for (auto& node : *m_rootNodes)
        {
            node->GetTransform()->MakeTransformChanged();
        }
    }

    void NodeCollection::EndScene()
    {
        for (auto& node : *m_nodes)
        {
            node->EndNode();
        }
        m_runtimeWorld = nullptr;
    }

    void NodeCollection::Tick(Ticker ticker)
    {
        for (auto& node : *m_nodes)
        {
            if (node && node->GetIsActive())
            {
                node->OnTick(ticker);
            }
        }
    }

    void NodeCollection::BeginPlay()
    {
        for (auto& node : *m_nodes)
        {
            if (node && node->GetIsActive())
            {
                node->BeginPlay();
            }
        }
    }

    void NodeCollection::EndPlay()
    {
        for (auto& node : *m_nodes)
        {
            if (node && node->GetIsActive())
            {
                node->EndPlay();
            }
        }
    }

    void NodeCollection::OnAddNode(ObjectPtr<Node> node)
    {
        if (m_runtimeWorld)
        {
            if (!node->GetOwnerNodeCollection())
            {
                node->BeginNode(this);
            }
        }
    }

    void NodeCollection::OnRemoveNode(ObjectPtr<Node> node)
    {
        if (m_runtimeWorld)
        {
            node->EndNode();
        }
    }

} // namespace pulsar