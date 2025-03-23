#include "Assets/NodeCollection.h"
#include <Pulsar/Node.h>

#include <utility>

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
                nodeObj->Add("Id", node->GetObjectHandle().to_string());

                NodeSerializer nodeSerializer{nodeObj, s->IsWrite, s->HasEditorData};
                node->Serialize(&nodeSerializer);
                nodes->Push(nodeObj);
            }
            s->Object->Add("Nodes", nodes);

            auto rootNodes = s->Object->New(ser::VarientType::Array);
            for (auto node : *m_rootNodes)
            {
                rootNodes->Push(node.GetHandle().to_string());
            }
            s->Object->Add("RootNodes", rootNodes);
        }
        else
        {
            auto nodesArr = s->Object->At("Nodes");
            for (int i = 0; i < nodesArr->GetCount(); ++i)
            {
                auto node = nodesArr->At(i);
                auto nodeId = ObjectHandle::parse(node->At("Id")->AsString());
                auto newNode = ConstructNode("Node", nodeId);
                NodeSerializer nodeSerializer{node, s->IsWrite, s->HasEditorData};
                newNode->Serialize(&nodeSerializer);
            }

            auto rootNodeArr = s->Object->At("RootNodes");
            for (int i = 0; i < rootNodeArr->GetCount(); ++i)
            {
                auto node = rootNodeArr->At(i);
                auto handle = ObjectHandle::parse(node->AsString());
                RegisterRootNode(handle);
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
    Node_ref NodeCollection::FindNodeByName(index_string name) const
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


    Node_ref NodeCollection::FindNodeByPath(string_view name) const
    {
        assert(false);
        return Node_ref();
    }
    void NodeCollection::OnInstantiateAsset(AssetObject* obj)
    {
        base::OnInstantiateAsset(obj);
        auto self = static_cast<NodeCollection*>(obj);

        
    }

    Node_ref NodeCollection::NewNode(index_string name, const Node_ref& parent, ObjectFlags flags)
    {
        auto node = BeginNewNode(name, parent, flags);
        EndNewNode(node);
        return node;
    }
    void NodeCollection::RemoveNode(Node_ref node)
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
                break;;
            }
        }

        DestroyObject(node, true);

        std::erase(*m_collectionHandles, node.GetHandle());
    }
    void NodeCollection::OnAddNode(ObjectPtr<Node> node)
    {
    }
    void NodeCollection::OnRemoveNode(ObjectPtr<Node> node)
    {
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

    ObjectPtr<Node> NodeCollection::ConstructNode(index_string name, ObjectHandle handle, ObjectFlags flags)
    {
        auto newNode = mksptr(new Node);
        newNode->Construct(handle);
        newNode->SetIndexName(name);
        newNode->SetObjectFlags(newNode->GetObjectFlags() | flags | OF_LifecycleManaged);

        auto node = ObjectPtr{newNode};

        m_nodes->push_back(node);

        return node;
    }

} // namespace pulsar