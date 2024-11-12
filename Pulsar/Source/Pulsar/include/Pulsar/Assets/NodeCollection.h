#pragma once
#include <Pulsar/AssetObject.h>


namespace pulsar
{
    class Node;
    class NodeCollection : public AssetObject
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::NodeCollection, AssetObject)
    public:
        void Serialize(AssetSerializer* s) override;

        void OnDestroy() override;

        const List_sp<ObjectPtr<Node>>& GetRootNodes() const { return m_rootNodes; }
        const List_sp<ObjectPtr<Node>>& GetNodes() const { return m_nodes; }

        ObjectPtr<Node> FindNodeByName(index_string name) const;
        ObjectPtr<Node> FindNodeByPath(string_view name) const;

        void OnInstantiateAsset(AssetObject* obj) override;


        ObjectPtr<Node> NewNode(index_string name = "Node", const ObjectPtr<Node>& parent = nullptr, ObjectFlags flags = 0);
        void RemoveNode(ObjectPtr<Node> node);

        virtual void OnAddNode(ObjectPtr<Node> node);
        virtual void OnRemoveNode(ObjectPtr<Node> node);

        void RegisterRootNode(const ObjectPtr<Node>& node);
        void UnregisterRootNode(const ObjectPtr<Node>& node);

        virtual void CopyFrom(ObjectPtr<NodeCollection> nc);
        const List_sp<ObjectHandle>& GetCollectionHandles() const { return m_collectionHandles; }

        NodeCollection();
    protected:
        ObjectPtr<Node> BeginNewNode(index_string name = "Node", const ObjectPtr<Node>& parent = nullptr, ObjectFlags flags = 0);
        void EndNewNode(ObjectPtr<Node> node);
    protected:
        CORELIB_REFL_DECL_FIELD(m_rootNodes);
        List_sp<ObjectPtr<Node>> m_rootNodes;

        CORELIB_REFL_DECL_FIELD(m_nodes);
        List_sp<ObjectPtr<Node>> m_nodes;

        CORELIB_REFL_DECL_FIELD(m_collectionHandles);
        List_sp<ObjectHandle> m_collectionHandles;
    };
    DECL_PTR(NodeCollection);

}