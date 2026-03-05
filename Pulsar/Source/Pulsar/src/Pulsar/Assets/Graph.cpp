#include "Assets/Graph.h"

namespace pulsar
{
    GraphPin* GraphNode::CreatePin(string_view name, GraphPinType type)
    {
        GraphPin* pin;
        if (type == GraphPinType::Input)
        {
            pin = m_inPins.emplace_back(std::make_unique<GraphPin>()).get();
        }
        else
        {
            pin = m_outPins.emplace_back(std::make_unique<GraphPin>()).get();
        }

        pin->m_name = name;
        pin->m_type = type;
        pin->m_node = this;

        return pin;
    }
    array_list<GraphPin*> GraphNode::GetAllPins() const
    {
        array_list<GraphPin*> result;
        for (auto& pin : m_inPins)
        {
            result.push_back(pin.get());
        }
        for (auto& pin : m_outPins)
        {
            result.push_back(pin.get());
        }
        return result;
    }
    GraphPin* GraphNode::GetInPinAt(int index) const
    {
        if (index >= 0 && index < m_inPins.size())
        {
            return m_inPins[index].get();
        }
        return nullptr;
    }
    GraphPin* GraphNode::GetOutPinAt(int index) const
    {
        if (index >= 0 && index < m_outPins.size())
        {
            return m_outPins[index].get();
        }
        return nullptr;
    }
    SPtr<GraphNode> Graph::NewNode(Type* type)
    {
        auto node = sptr_cast<GraphNode>(type->CreateSharedInstance({}));
        node->m_graph = this;
        node->m_guid = guid_t::create_new();
        m_nodes.push_back(node);

        return node;
    }

    void Graph::MakeLink(GraphPin* a, GraphPin* b)
    {
        auto& link = m_linked.emplace_back();
        link.NodeA = a;
        link.NodeB = b;
        static uint64_t linkId = 0;
        ++linkId;
        link.Id = linkId;

        a->m_linked.push_back(b);
        b->m_linked.push_back(a);
    }
    void Graph::BreakLink(GraphPin* a, GraphPin* b)
    {
        for (auto it = m_linked.begin(); it != m_linked.end(); ++it)
        {
            if (it->NodeA == a && it->NodeB == b)
            {
                std::erase(a->m_linked, b);
                std::erase(b->m_linked, a);
                m_linked.erase(it);
                break;
            }
        }
    }
    void Graph::BreakLinkById(uint64_t id)
    {
        std::erase_if(m_linked, [id](auto& linked) {return linked.Id == id; });
    }
} // namespace pulsar