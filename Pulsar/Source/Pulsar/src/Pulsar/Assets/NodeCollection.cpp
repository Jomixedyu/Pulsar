#include "Assets/NodeCollection.h"
#include <Pulsar/Node.h>
#include <Pulsar/World.h>
#include <Pulsar/AssetManager.h>

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
                // 来自 Prefab 实例化的节点不保存，BeginComponent 时重新实例化
                if (node->IsTemplateInstance())
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

            // 保存模板实例列表（只存 asset GUID）
            auto templatesArr = s->Object->New(ser::VarientType::Array);
            for (auto& inst : m_templateInstances)
            {
                if (inst.Template)
                    templatesArr->Push(inst.Template.GetGuid().to_string());
            }
            s->Object->Add("TemplateInstances", templatesArr);
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
                AddSceneObjectToFinder(nodeId, newNode);
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

            // 读取模板实例列表，并在此处直接展开（迭代外，安全）
            if (auto templatesArr = s->Object->At("TemplateInstances"))
            {
                for (int i = 0; i < templatesArr->GetCount(); ++i)
                {
                    auto tmplGuidStr = templatesArr->At(i)->AsString();
                    auto tmplGuid = guid_t::parse(tmplGuidStr);
                    auto tmpl = AssetManager::Get()->LoadAssetById<NodeCollection>(tmplGuid);
                    if (tmpl)
                    {
                        TemplateInstanceInfo info;
                        info.Template = tmpl;
                        info.RootNodes = CombineFrom(tmpl);
                        m_templateInstances.push_back(std::move(info));
                    }
                }
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
        node->m_ownerCollection = this;   // 设置静态归属，IStringify_Stringify 能正确找到资产 GUID
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

    void NodeCollection::AddSceneObjectToFinder(const guid_t& guid, const ObjectPtr<SceneObject>& obj)
    {
        m_guidToNode.insert({guid, obj});
    }



    void NodeCollection::AddTemplateInstance(RCPtr<NodeCollection> tmpl)
    {
        TemplateInstanceInfo info;
        info.Template = tmpl;
        info.RootNodes = CombineFrom(tmpl);
        m_templateInstances.push_back(std::move(info));
    }

    void NodeCollection::RemoveTemplateInstance(int index)
    {
        assert(index >= 0 && index < (int)m_templateInstances.size());
        auto& info = m_templateInstances[index];
        for (auto& root : info.RootNodes)
        {
            if (root)
                RemoveNode(root);
        }
        m_templateInstances.erase(m_templateInstances.begin() + index);
    }

    int NodeCollection::FindTemplateInstanceIndex(ObjectPtr<Node> node) const
    {
        if (!node->IsTemplateInstance())
            return -1;

        // 向上找到最顶层的 template 节点（parent 不再是 template 节点时即为根）
        ObjectPtr<Node> root = node;
        while (true)
        {
            auto parentTransform = root->GetTransform()->GetParent();
            if (!parentTransform)
                break;
            auto parentNode = parentTransform->GetNode();
            if (!parentNode->IsTemplateInstance())
                break;
            root = parentNode;
        }

        // 在 m_templateInstances 里找哪个实例的 RootNodes 包含这个根节点
        for (int i = 0; i < (int)m_templateInstances.size(); ++i)
        {
            for (auto& r : m_templateInstances[i].RootNodes)
            {
                if (r.GetHandle() == root.GetHandle())
                    return i;
            }
        }
        return -1;
    }

    void NodeCollection::RemoveTemplateInstanceByNode(ObjectPtr<Node> node)
    {
        int index = FindTemplateInstanceIndex(node);
        if (index >= 0)
            RemoveTemplateInstance(index);
    }

    array_list<ObjectPtr<Node>> NodeCollection::CombineFrom(RCPtr<NodeCollection> collection)
    {
        // 临时 finder：oldGuid → 新建对象（节点和组件）
        struct TempFinder : ISceneObjectFinder
        {
            hash_map<guid_t, ObjectPtr<SceneObject>> map;

            ObjectPtr<SceneObject> FindSceneObject(guid_t id) const override
            {
                auto it = map.find(id);
                return it != map.end() ? it->second : ObjectPtr<SceneObject>{};
            }
            void AddSceneObjectToFinder(const guid_t& guid, const ObjectPtr<SceneObject>& obj) override
            {
                map[guid] = obj;
            }
        } tempFinder;

        // Step 1: 预构建所有新节点和组件，建立 oldGuid → 新对象 的映射
        struct NodePair
        {
            ObjectPtr<Node> SrcNode;
            ObjectPtr<Node> NewNode;
        };
        array_list<NodePair> pairs;

        for (auto& srcNode : *collection->GetNodes())
        {
            // 创建新节点（新 GUID，自动加入 m_nodes）
            auto newNode = ConstructNode(srcNode->GetName());
            newNode->m_sourceGuidInTemplate = srcNode->GetSceneObjectGuid();

            // 映射：旧节点 GUID → 新节点
            tempFinder.AddSceneObjectToFinder(srcNode->GetSceneObjectGuid(), newNode);

            pairs.push_back({ srcNode, newNode });
        }

        // Step 2: 复制节点字段 + 创建组件，注册进 tempFinder
        for (auto& [srcNode, newNode] : pairs)
            newNode->BeginInstantiate(srcNode, &tempFinder);

        // Step 3: 反序列化 component 数据（需要完整映射表）
        for (auto& [srcNode, newNode] : pairs)
            newNode->EndInstantiate(srcNode, &tempFinder);

        // Step 3.5: 反序列化完成后，标记所有 transform matrix 为 dirty
        // 因为 m_isDirtyMatrix 默认为 false，反序列化不会改变它，
        // 导致 GetLocalToWorldMatrix() 返回未初始化的 identity 矩阵
        for (auto& [srcNode, newNode] : pairs)
        {
            if (auto t = newNode->GetTransform())
                t->MakeTransformChanged();
        }

        // Step 4: 注册所有新节点进 this 的 finder，确定 rootNodes
        for (auto& [srcNode, newNode] : pairs)
        {
            AddSceneObjectToFinder(newNode->GetSceneObjectGuid(), newNode);

            // 同时注册该节点的组件
            for (auto& comp : newNode->GetAllComponentArray())
            {
                AddSceneObjectToFinder(comp->GetSceneObjectGuid(), comp);
            }
        }

        // 没有 parent 的节点作为 root
        for (auto& [srcNode, newNode] : pairs)
        {
            if (newNode->GetTransform()->GetParent() == nullptr)
            {
                RegisterRootNode(newNode);
            }
        }

        // 收集本次展开的根节点
        array_list<ObjectPtr<Node>> newRoots;
        for (auto& [srcNode, newNode] : pairs)
        {
            if (newNode->GetTransform()->GetParent() == nullptr)
                newRoots.push_back(newNode);
        }

        // Step 5: 如果场景正在运行，启动所有新节点（BeginNode → BeginComponent → 渲染注册）
        // 所有节点都处理完后再统一 Begin，确保组件初始化时引用已全部就绪
        for (auto& [srcNode, newNode] : pairs)
        {
            OnAddNode(newNode);
        }

        return newRoots;
    }

    void NodeCollection::OnCollectAssetDependencies(array_list<guid_t>& deps)
    {
        for (auto& node : *m_nodes)
        {
            if (node->IsTransientObject())
            {
                continue;
            }
            if (node->IsTemplateInstance())
            {
                continue;
            }
            node->GetDependenciesAsset(deps);
        }
        // 模板资产依赖
        for (auto& inst : m_templateInstances)
        {
            if (inst.Template)
                deps.push_back(inst.Template.GetGuid());
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
        // 拷贝后再遍历，防止 EndComponent 回调链中修改 m_nodes 导致迭代器失效
        auto nodesCopy = *m_nodes;
        for (auto& node : nodesCopy)
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
            if (!node->m_isBegun)  // 检查是否已经 Begin，而非是否有归属
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