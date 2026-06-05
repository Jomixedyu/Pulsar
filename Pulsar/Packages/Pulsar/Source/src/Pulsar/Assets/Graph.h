#pragma once
#include "Pulsar/AssetObject.h"

namespace pulsar
{
    class GraphNode;
    class Graph;

    enum class GraphPinType
    {
        Input,
        Output
    };

    class GraphPin final
    {
    public:
        string m_name;
        bool m_isHidden;

        Type* m_dataType;
        array_list<GraphPin*> m_linked;
        GraphPinType m_type;
        GraphNode* m_node;
    };


    class GraphNode : public Object
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::GraphNode, Object);
    public:
        GraphPin* CreatePin(string_view name, GraphPinType type);
    public:

        array_list<GraphPin*> GetAllPins() const;
        const auto& GetInPins() const { return m_inPins; }
        const auto& GetOutPins() const { return m_outPins; }
        GraphPin* GetInPinAt(int index) const;
        GraphPin* GetOutPinAt(int index) const;

        virtual string_view GetTitle() const { return GetType()->GetShortName(); }

        array_list<std::unique_ptr<GraphPin>> m_inPins;
        array_list<std::unique_ptr<GraphPin>> m_outPins;

        Vector2f m_position{};
        string m_comment;

        guid_t m_guid;
        Graph* m_graph = nullptr;

    };

    struct GraphLinked
    {
        GraphPin* NodeA;
        GraphPin* NodeB;
        uint64_t Id;
    };

    class Graph : public AssetObject
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::Graph, AssetObject);

    public:
        SPtr<GraphNode> NewNode(Type* type);

        const array_list<SPtr<GraphNode>>& GetNodes() const
        {
            return m_nodes;
        }

        template <typename T>
            requires std::is_base_of_v<GraphNode, T>
        SPtr<T> NewNode()
        {
            return sptr_cast<T>(NewNode(cltypeof<T>()));
        }

        void MakeLink(GraphPin* a, GraphPin* b);
        void BreakLink(GraphPin* a, GraphPin* b);
        void BreakLinkById(uint64_t id);

        const auto& GetLinked() const
        {
            return m_linked;
        }

    protected:
        array_list<SPtr<GraphNode>> m_nodes;

        array_list<GraphLinked> m_linked;
    };

} // namespace pulsar