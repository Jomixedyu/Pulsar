#pragma once

#include <vector>
#include <list>
#include <type_traits>

#include <Pulsar/ObjectBase.h>
#include <Pulsar/AssetObject.h>
#include <Pulsar/Components/Component.h>
#include <Pulsar/Components/TransformComponent.h>
#include <Pulsar/Ticker.h>

namespace pulsar
{
    using namespace jxcorlib;


    using MessageType_t = uint32_t;
    namespace MessageType
    {
        constexpr inline MessageType_t Initialize = 1;
        constexpr inline MessageType_t Destory = 2;
        constexpr inline MessageType_t Update = 3;

        constexpr inline MessageType_t Custom = 1000;
    }

    template<typename T>
    concept baseof_component_concept = std::is_base_of<Component, T>::value;

    class Scene;
    class World;
    class NodeCollection;

    struct NodeSerializer
    {
        NodeSerializer(ser::VarientRef obj, bool isWrite, bool editorData)
            : Object(std::move(obj)),
              IsWrite(isWrite),
              HasEditorData(editorData)
        {
        }

        NodeSerializer(const NodeSerializer&) = delete;
        NodeSerializer(NodeSerializer&&) = delete;
    public:
        ser::VarientRef Object;
        const bool IsWrite;
        const bool HasEditorData;
    };

    class Node final : public ObjectBase, public ITickable
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::Node, ObjectBase);
        ObjectPtr<Node> self_ref() const { return GetObjectHandle(); }
    public:
        void Serialize(NodeSerializer* s);
        bool GetIsActive() const;
        bool GetIsActiveSelf() const { return m_active; }
        void SetIsActiveSelf(bool value);

        ObjectPtr<Node> GetParent() const;
        void SetParent(ObjectPtr<Node> parent);

        void OnActive();
        void OnInactive();
        void OnParentActiveChanged();

        TransformComponent_ref GetTransform() const;
    public:
        Node(const Node& r) = delete;
        Node(Node&& r) = delete;
        Node& operator=(const Node& r) = delete;
    public:
        Node();
        ~Node() override;
        void SendMessage(MessageId id);

        //ITickable 
        virtual void OnTick(Ticker ticker) override;
    public:
        virtual void OnConstruct() override;
        virtual void OnDestroy() override;

        void BeginNode(ObjectPtr<Scene> scene);
        void EndNode();

    public: //components
        template<typename T>
        ObjectPtr<T>         AddComponent() { return this->AddComponent(cltypeof<T>()); }
        ObjectPtr<Component> AddComponent(Type* type);
        void                 DestroyComponent(ObjectPtr<Component> component);
        int                  IndexOf(ObjectPtr<Component> component) const;

        template<baseof_component_concept T>
        ObjectPtr<T>         GetComponent() { return this->GetComponent(cltypeof<T>()); }
        ObjectPtr<Component> GetComponent(Type* type) const;


        void                             GetAllComponents(array_list<ObjectPtr<Component>>& list);
        array_list<ObjectPtr<Component>> GetAllComponentArray() const;
        size_t                           GetComponentCount() const { return this->m_components->size(); }

    protected:
        void BeginComponent(Component_ref component);
        void EndComponent(Component_ref component);
    public:
        ObjectPtr<Scene> GetRuntimeOwnerScene() const { return m_runtimeScene; }
        World* GetRuntimeWorld() const;
    private:

        CORELIB_REFL_DECL_FIELD(m_active);
        bool m_active = true;

        CORELIB_REFL_DECL_FIELD(m_components);
        List_sp<ObjectPtr<Component>> m_components;

        bool m_isInitialized = false;

        ObjectPtr<Scene> m_runtimeScene = nullptr;

        ObjectPtr<NodeCollection> m_owner;
    };
    DECL_PTR(Node);

}
