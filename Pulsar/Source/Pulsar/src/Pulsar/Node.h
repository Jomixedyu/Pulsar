#pragma once

#include <vector>
#include <list>
#include <type_traits>


#include <Pulsar/ObjectBase.h>
#include <Pulsar/SceneObject.h>
#include <Pulsar/AssetObject.h>
#include <Pulsar/Components/Component.h>
#include <Pulsar/Components/TransformComponent.h>
#include <Pulsar/Ticker.h>

namespace pulsar
{
    using namespace jxcorlib;

    template<typename T>
    concept baseof_component_concept = std::is_base_of<Component, T>::value;

    class Scene;
    class World;
    class NodeCollection;
    class Component;
    class TransformComponent;



    class Node final : public SceneObject, public ITickable
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::Node, SceneObject);
        DECL_OBJECTPTR_SELF

        friend class NodeCollection;
    public:
        void BeginSerialize(SceneObjectSerializer* s);
        void EndSerialize(SceneObjectSerializer* s);
        bool GetIsActive() const;
        bool GetIsActiveSelf() const
        {
            return m_active;
        }
        void SetIsActiveSelf(bool value);

        ObjectPtr<Node> GetParent() const;
        void SetParent(ObjectPtr<Node> parent);

        void OnActive();
        void OnInactive();
        void OnParentActiveChanged();
        void OnTransformChanged();
        ObjectPtr<TransformComponent> GetTransform() const;
        BoxSphereBounds3f GetBounds();
        void GetDependenciesAsset(array_list<jxcorlib::guid_t> &deps) const override;
    public:
        Node(const Node& r) = delete;
        Node(Node&& r) = delete;
        Node& operator=(const Node& r) = delete;

    public:
        Node();
        ~Node() override;
        void SendMessage(MessageId id);

        // ITickable
        virtual void OnTick(Ticker ticker) override;

    public:
        virtual void OnConstruct() override;
        virtual void OnDestroy() override;

        void BeginNode(NodeCollection* collection);
        void EndNode();

        void BeginPlay();
        void EndPlay();



    public: // components
        template <baseof_component_concept T>
        ObjectPtr<T> AddComponent()
        {
            return cast<T>( this->AddComponent(cltypeof<T>()) );
        }
        ObjectPtr<Component> AddComponent(Type* type);
        void DestroyComponent(ObjectPtr<Component> component);
        int IndexOf(ObjectPtr<Component> component) const;

        template <baseof_component_concept T>
        ObjectPtr<T> GetComponent()
        {
            return cast<T>(this->GetComponent(cltypeof<T>()));
        }
        ObjectPtr<Component> GetComponent(Type* type) const;

        void GetAllComponents(array_list<ObjectPtr<Component>>& list);
        const array_list<ObjectPtr<Component>>& GetAllComponentArray() const;
        size_t GetComponentCount() const
        {
            return this->m_components->size();
        }

        template <baseof_component_concept T>
        void GetComponents(array_list<ObjectPtr<T>>& array) const
        {
            for (const auto& item : *this->m_components)
            {
                if (auto comp = cast<T>(item))
                {
                    array.push_back(comp);
                }
            }
        }

        template <baseof_component_concept T>
        void GetComponentsInChildren(array_list<ObjectPtr<T>>& array) const
        {
            GetComponents(array);
            for (const auto& item : *GetTransform()->GetChildren())
            {
                item->GetNode()->GetComponentsInChildren(array);
            }
        }

        auto GetLayer() const
        {
            return this->m_layer;
        }
        void SetLayer(int32_t layer);

    protected:
        ObjectPtr<Component> ConstructComponent(Type* type, const guid_t& guid = {});
        void BeginComponent(Component_ref component);
        void EndComponent(Component_ref component);

    public:
        virtual NodeCollection* GetOwnerNodeCollection() const override
        {
            return m_runtimeCollection;
        }
        World* GetRuntimeWorld() const;

    private:
        CORELIB_REFL_DECL_FIELD(m_active);
        bool m_active = true;

        CORELIB_REFL_DECL_FIELD(m_components);
        List_sp<ObjectPtr<Component>> m_components;

        ObjectPtr<TransformComponent> m_transform = nullptr;

        bool m_isInitialized = false;

        NodeCollection* m_runtimeCollection = nullptr;

        ObjectPtr<NodeCollection> m_owner;

        int32_t m_layer = 0;
    };

}
