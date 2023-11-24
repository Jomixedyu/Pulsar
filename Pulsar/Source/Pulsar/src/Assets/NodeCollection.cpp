#include "Assets/NodeCollection.h"

namespace pulsar
{
    Node_ref NodeCollection::FindNodeByName(string_view name) const
    {
        return Node_ref();
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
    void NodeCollection::RemoveNode(Node_ref node)
    {
        
    }

    NodeCollection::NodeCollection()
    {
        m_rootNodes = mksptr(new List<Node_ref>);
        m_nodes = mksptr(new List<Node_ref>);
    }

}