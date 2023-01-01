#pragma once

#include <vector>
#include <list>
#include <type_traits>

#include <Apatite/ObjectBase.h>
#include <Apatite/Math.h>
#include <Apatite/AssetObject.h>
#include <Apatite/Components/Component.h>

namespace apatite
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

    class Node : public AssetObject
    {
        CORELIB_DEF_TYPE(AssemblyObject_Apatite, apatite::Node, AssetObject);
    public:
        bool get_is_active() const;
        bool get_is_active_self() const;
        void set_is_active_self(bool value);

        wptr<Node> get_parent() const;
        void set_parent(wptr<Node> parent, bool keep_world_transform = false);

        int32_t get_child_count() const;
    public:
        Node(const Node& r) = delete;
        Node(Node&& r) = delete;
        Node& operator=(const Node& r) = delete;
    public:
        Node() {};

        virtual void Serialize(ser::Stream& stream, bool is_ser) override; 
    public:
        //void BroadcastSendMessage(MessageType_t msg);
        //virtual void SendMessage(MessageType_t msg);
        virtual void OnInitialize();
        virtual void OnDestory();
        virtual void OnUpdate();
    public:
        sptr<Component> AddComponent(Type* type);
        template<typename T>
        sptr<T> AddComponent()
        {
            return sptr_cast<T>(this->AddComponent(cltypeof<T>()));
        }

        sptr<Component> GetComponent(Type* type) const;

        template<baseof_component_concept T>
        sptr<T> GetComponent()
        {
            return sptr_cast<T>(this->GetComponent(cltypeof<T>()));
        }

        void GetAllComponents(List_sp<sptr<Component>>& list);

        void GetChildren(List_sp<sptr<Node>>& list);

        sptr<Node> GetChild(string_view name);
        sptr<Node> GetChildAt(int index);
    protected:
        virtual void OnInstantiateAsset(sptr<AssetObject>& obj) override;
    public:
        void Translate(Vector3f v);
        void Rotate(Quat4f v);
        void RotateEuler(Vector3f v);
        void Scale(Vector3f v);
        Matrix4f GetLocalMatrix() const;
        Matrix4f GetWorldMatrix() const;
    private:
        bool is_active_;
        wptr<Node> parent_;

        CORELIB_REFL_DECL_FIELD(position_);
        Vector3f position_;
        CORELIB_REFL_DECL_FIELD(rotation_);
        Quat4f rotation_;
        CORELIB_REFL_DECL_FIELD(scale_);
        Vector3f scale_;

        CORELIB_REFL_DECL_FIELD(childs_);
        List_sp<sptr<Node>> childs_;

        CORELIB_REFL_DECL_FIELD(components_);
        List_sp<sptr<Component>> components_;
    };
    CORELIB_DECL_SHORTSPTR(Node);

}
