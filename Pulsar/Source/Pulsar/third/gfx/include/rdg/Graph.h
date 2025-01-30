#pragma once
#include "Node.h"

#include <memory>
#include <unordered_map>

namespace rdg
{
    class Graph
    {
    public:
        template<typename T> requires std::is_base_of_v<Node, T>
        std::shared_ptr<Node> NewNode()
        {
            auto node = std::make_shared<T>();
            node->Initialize(this, GenerateId());
            return node;
        }
        void MakeLink(Node::NodeId src, Node::NodeId dst);
        int GetRefCount(Node::NodeId id) const;
        bool IsCulled(Node::NodeId id) const;
        void SetTarget(Node::NodeId id);
        Node::NodeId GetTarget() const { return m_target;}
    protected:
        static Node::NodeId GenerateId();
    protected:
        Node::NodeId m_target{};
        std::unordered_map<Node::NodeId, std::shared_ptr<Node>> m_nodes;
        std::unordered_map<Node::NodeId, std::vector<Node::NodeId>> m_dependencies;
    };
}