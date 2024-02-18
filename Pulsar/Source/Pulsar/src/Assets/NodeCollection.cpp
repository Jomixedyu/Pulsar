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
    }
    void NodeCollection::OnAddNode(ObjectPtr<Node> node)
    {
    }
    void NodeCollection::OnRemoveNode(ObjectPtr<Node> node)
    {
    }

    void NodeCollection::CopyFrom(ObjectPtr<NodeCollection> nc)
    {


    }

    NodeCollection::NodeCollection()
    {
        m_rootNodes = mksptr(new List<Node_ref>);
        m_nodes = mksptr(new List<Node_ref>);
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
        return node;
    }
    void NodeCollection::EndNewNode(ObjectPtr<Node> node)
    {
        OnAddNode(node);
    }

} // namespace pulsar