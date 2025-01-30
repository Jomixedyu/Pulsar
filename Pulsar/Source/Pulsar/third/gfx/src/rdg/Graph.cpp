#include "rdg/Graph.h"

namespace rdg
{


    void Graph::MakeLink(Node::NodeId src, Node::NodeId dst)
    {
        m_dependencies[src].push_back(dst);
    }
    int Graph::GetRefCount(Node::NodeId id) const
    {
        return m_dependencies.at(id).size();
    }
    bool Graph::IsCulled(Node::NodeId id) const
    {
        return GetRefCount(id) == 0;
    }
    void Graph::SetTarget(Node::NodeId id)
    {
        m_target = id;
    }

    Node::NodeId Graph::GenerateId()
    {
        static Node::NodeId id = 0;
        ++id;
        return id;
    }
} // namespace rdg