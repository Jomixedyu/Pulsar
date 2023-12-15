#pragma once
#include <Pulsar/AssetObject.h>
#include <Pulsar/Node.h>

namespace pulsar
{
    class NodeCollection : public AssetObject
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::NodeCollection, AssetObject)
    public:
        const List_sp<Node_ref>& GetRootNodes() const { return m_rootNodes; }
        const List_sp<Node_ref>& GetNodes() const { return m_nodes; }

        Node_ref FindNodeByName(string_view name) const;
        Node_ref FindNodeByPath(string_view name) const;

        void AddNode(Node_ref node);
        Node_ref NewNode(string_view name = "Node");
        void RemoveNode(Node_ref node);

        virtual void OnAddNode(Node_ref node) {}
        virtual void OnRemoveNode(Node_ref node) {}

        NodeCollection();
    protected:
        CORELIB_REFL_DECL_FIELD(m_rootNodes);
        List_sp<Node_ref> m_rootNodes;

        CORELIB_REFL_DECL_FIELD(m_nodes);
        List_sp<Node_ref> m_nodes;
    };
}