#include "rdg/Node.h"

namespace rdg
{

    void Node::Initialize(Graph* graph, NodeId id)
    {
        m_graph = graph;
        m_nodeId = id;
    }
} // namespace rdg