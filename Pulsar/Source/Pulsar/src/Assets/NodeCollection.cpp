#include "Assets/NodeCollection.h"

namespace pulsar
{
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

    void NodeCollection::AddNode(Node_ref node)
    {
        m_nodes->push_back(node);
        if (node->GetTransform()->GetParent() == nullptr)
        {
            m_rootNodes->push_back(node);
        }

        node->GetTransform()->GetChildren();

        OnAddNode(node);
    }
    Node_ref NodeCollection::NewNode(string_view name)
    {
        auto node = Node::StaticCreate(name);
        m_nodes->push_back(node);
        m_rootNodes->push_back(node);
        OnAddNode(node);
        return node;
    }
    void NodeCollection::RemoveNode(Node_ref node)
    {
        
    }

    NodeCollection::NodeCollection()
    {
        m_rootNodes = mksptr(new List<Node_ref>);
        m_nodes = mksptr(new List<Node_ref>);
    }

}