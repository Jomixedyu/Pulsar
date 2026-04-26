#pragma once
#include <Pulsar/AssetObject.h>
#include <Pulsar/SceneObject.h>
#include <Pulsar/Ticker.h>
#include <Pulsar/SceneEnvironment.h>

namespace pulsar
{
    class Node;
    class World;
    // 记录一个模板实例化条目，序列化进 NodeCollection
    struct TemplateInstanceInfo
    {
        RCPtr<NodeCollection> Template;
        array_list<ObjectPtr<Node>> RootNodes; // 该实例展开后的根节点（不序列化，运行时维护）
        // 未来可以加 overrides
    };

    class NodeCollection : public AssetObject, public ISceneObjectFinder
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::NodeCollection, AssetObject)
    public:

        virtual ObjectPtr<SceneObject> FindSceneObject(guid_t sceneObjId) const override;
        virtual void AddSceneObjectToFinder(const guid_t& guid, const ObjectPtr<SceneObject>& obj) override;

        void Serialize(AssetSerializer* s) override;

        void OnDestroy() override;

        const List_sp<ObjectPtr<Node>>& GetRootNodes() const { return m_rootNodes; }
        const List_sp<ObjectPtr<Node>>& GetNodes() const { return m_nodes; }

        ObjectPtr<Node> FindNodeByName(index_string name) const;
        ObjectPtr<Node> FindNodeByPath(string_view name) const;

        void OnInstantiateAsset(AssetObject* obj) override;

        // 返回本次展开的根节点列表
        array_list<ObjectPtr<Node>> CombineFrom(RCPtr<NodeCollection> collection);

        // 添加模板实例（记录引用并立即展开节点）
        void AddTemplateInstance(RCPtr<NodeCollection> tmpl);

        // 按索引移除模板实例（同时销毁所有实例化出的节点）
        void RemoveTemplateInstance(int index);

        // 传入该实例中的任意节点，自动找到所属实例并移除
        void RemoveTemplateInstanceByNode(ObjectPtr<Node> node);

        // 找到节点所属的模板实例索引，未找到返回 -1
        int FindTemplateInstanceIndex(ObjectPtr<Node> node) const;

        const array_list<TemplateInstanceInfo>& GetTemplateInstances() const { return m_templateInstances; }

        ObjectPtr<Node> NewNode(index_string name = "Node", const ObjectPtr<Node>& parent = nullptr, ObjectFlags flags = 0);
        void RemoveNode(ObjectPtr<Node> node);

        virtual void OnAddNode(ObjectPtr<Node> node);
        virtual void OnRemoveNode(ObjectPtr<Node> node);

        void RegisterRootNode(const ObjectPtr<Node>& node);
        void UnregisterRootNode(const ObjectPtr<Node>& node);

        const List_sp<ObjectHandle>& GetCollectionHandles() const { return m_collectionHandles; }

        void OnCollectAssetDependencies(array_list<jxcorlib::guid_t> &deps) override;

        // 运行时生命周期
        virtual void BeginScene(World* world);
        virtual void EndScene();
        virtual void Tick(Ticker ticker);
        virtual void BeginPlay();
        virtual void EndPlay();

        virtual SceneRuntimeEnvironment* GetRuntimeEnvironment() { return nullptr; }

        World* GetWorld() const { return m_runtimeWorld; }

        NodeCollection();
    protected:
        ObjectPtr<Node> BeginNewNode(index_string name = "Node", const ObjectPtr<Node>& parent = nullptr, ObjectFlags flags = 0);
        void EndNewNode(ObjectPtr<Node> node);
        ObjectPtr<Node> ConstructNode(index_string name = "Node", guid_t guid = {}, ObjectFlags flags = 0);
    protected:
        World* m_runtimeWorld = nullptr;
        CORELIB_REFL_DECL_FIELD(m_rootNodes);
        List_sp<ObjectPtr<Node>> m_rootNodes;

        CORELIB_REFL_DECL_FIELD(m_nodes);
        List_sp<ObjectPtr<Node>> m_nodes;

        CORELIB_REFL_DECL_FIELD(m_collectionHandles);
        List_sp<ObjectHandle> m_collectionHandles;

        hash_map<guid_t, ObjectPtr<SceneObject>> m_guidToNode;

        // 模板实例列表，序列化进场景文件
        array_list<TemplateInstanceInfo> m_templateInstances;
    };
    DECL_PTR(NodeCollection);

}