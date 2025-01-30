#pragma once
#include <cstdint>

namespace rdg
{
    class Graph;

    class Node
    {
    public:
        using NodeId = uint64_t;

        Node() {}
        virtual ~Node() = default;
        Node(const Node&) = delete;
        Node(Node&&) = delete;
        Node& operator=(const Node&) = delete;

        void Initialize(Graph* graph, NodeId id);

        NodeId GetId() const { return m_nodeId; }
        virtual const char* GetName() const { return "Node"; }

    protected:
        Graph* m_graph = nullptr;
        NodeId m_nodeId = 0;
    };
}