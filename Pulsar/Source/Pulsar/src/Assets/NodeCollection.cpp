#include "Assets/NodeCollection.h"
#include <Pulsar/Node.h>

namespace pulsar
{
    void NodeCollection::Serialize(AssetSerializer* s)
    {
        base::Serialize(s);
        if (s->IsWrite)
        {
            auto nodes = s->Object->New(ser::VarientType::Array);
            for (auto node : *m_nodes)
            {
                auto nodeObj = s->Object->New(ser::VarientType::Object);
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
    }
    Node_ref NodeCollection::FindNodeByName(index_string name) const
    {
        for (const auto node : *m_rootNodes)
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
        return Node_ref();
    }

    Node_ref NodeCollection::NewNode(index_string name, Node_ref parent, ObjectFlags flags)
    {
        auto node = BeginNewNode(name, parent, flags);
        EndNewNode(node);
        return node;
    }
    void NodeCollection::RemoveNode(Node_ref node)
    {
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
    ObjectPtr<Node> NodeCollection::BeginNewNode(index_string name, ObjectPtr<Node> parent, ObjectFlags flags)
    {
        auto newNode = mksptr(new Node);
        newNode->Construct();
        newNode->SetIndexName(name);
        newNode->SetObjectFlags(newNode->GetObjectFlags() | flags);

        auto node = ObjectPtr{newNode};

        m_nodes->push_back(node);
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

} // namespace pulsar