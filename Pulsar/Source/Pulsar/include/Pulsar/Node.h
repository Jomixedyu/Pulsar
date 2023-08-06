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

    class Node : public AssetObject, public ITickable
    {
        CORELIB_DEF_TYPE(AssemblyObject_Pulsar, pulsar::Node, AssetObject);
        ObjectPtr<Node> self_ref() const { return GetObjectHandle(); }
    public:
        bool get_is_active() const;
        bool get_is_active_self() const;
        void set_is_active_self(bool value);
        //TransformComponent_sp get_transform();

        bool has_parent() const { return this->has_parent_; }
        ObjectPtr<Node> GetParent() const;
        void SetParent(ObjectPtr<Node> parent, bool keepWorldTransform = false);
        ObjectPtr<Node> NewChildNode(string_view name);

        int32_t get_child_count() const;
    public:
        Vector3f get_self_position() const { return this->position_; }
        void set_self_position(Vector3f value) { this->position_ = value; }
        Vector3f get_world_position() const;

        Quat4f get_self_rotation() const { return this->rotation_; }
        void set_self_rotation(const Quat4f& value) { this->rotation_ = value; }
        Quat4f get_world_rotation() const;
        void set_world_rotation(const Quat4f& q);

        Vector3f get_self_euler_rotation() const;
        void set_self_euler_rotation(const Vector3f& value);
        Vector3f get_world_euler_rotation() const;

        Vector3f get_self_scale() const { return this->scale_; }
        void set_self_scale(Vector3f value) { this->scale_ = value; }
        Vector3f get_world_scale() const;


        Matrix4f GetModelMatrix() const;
        Matrix4f GetLocalTransformMatrix() const;
        Vector3f GetForward() const;
    public:
        Node(const Node& r) = delete;
        Node(Node&& r) = delete;
        Node& operator=(const Node& r) = delete;
    public:
        Node();
        virtual ~Node() override
        {
            int a = 3;
        }
        void SendMessage(MessageId id);

        //ITickable 
        virtual void OnTick(Ticker ticker) override;
    public:
        virtual void OnConstruct() override;
        virtual void OnDestroy() override;
    public:
        ObjectPtr<Component> AddComponent(Type* type);
        template<typename T>
        ObjectPtr<T> AddComponent()
        {
            return this->AddComponent(cltypeof<T>());
        }

        ObjectPtr<Component> GetComponent(Type* type) const;

        template<baseof_component_concept T>
        ObjectPtr<T> GetComponent()
        {
            return this->GetComponent(cltypeof<T>());
        }

        void GetAllComponents(List_sp<ObjectPtr<Component>>& list);
        array_list<ObjectPtr<Component>> GetAllComponentArray() const;
        size_t GetComponentCount() const { return this->m_components->size(); }
        void GetChildren(List_sp<ObjectPtr<Node>>& list);
        array_list<ObjectPtr<Node>> GetChildrenArray();

        ObjectPtr<Node> GetChild(string_view name);
        ObjectPtr<Node> GetChildAt(int index);
    public:
        static ObjectPtr<Node> StaticCreate(string_view name);
    protected:
        virtual void OnInstantiateAsset(ObjectPtr<AssetObject>& obj) override;
    public:

        void RotateEulerLocal(Vector3f v);
        void RotateEulerWorld(Vector3f v);

    private:
        bool is_active_;
        ObjectPtr<Node> m_parent;
        bool has_parent_ = false;

        CORELIB_REFL_DECL_FIELD(position_);
        Vector3f position_;
        CORELIB_REFL_DECL_FIELD(rotation_);
        Quat4f rotation_;
        CORELIB_REFL_DECL_FIELD(scale_);
        Vector3f scale_{ 1.f,1.f,1.f };

        CORELIB_REFL_DECL_FIELD(m_children);
        List_sp<ObjectPtr<Node>> m_children;

        CORELIB_REFL_DECL_FIELD(m_components);
        List_sp<ObjectPtr<Component>> m_components;

    };
    DECL_PTR(Node);

}
