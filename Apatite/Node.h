#pragma once

#include <vector>
#include <list>
#include <type_traits>

#include <Apatite/ObjectBase.h>
#include <Apatite/Math.h>
#include <Apatite/AssetObject.h>
#include <Apatite/Components/Component.h>
#include <Apatite/Components/TransformComponent.h>
#include <Apatite/Ticker.h>

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

    class Node : public AssetObject, public ITickable
    {
        CORELIB_DEF_TYPE(AssemblyObject_Apatite, apatite::Node, AssetObject);
    public:
        bool get_is_active() const;
        bool get_is_active_self() const;
        void set_is_active_self(bool value);
        //TransformComponent_sp get_transform();

        wptr<Node> get_parent() const;
        void set_parent(wptr<Node> parent, bool keep_world_transform = false);

        int32_t get_child_count() const;
    public:
        Vector3f get_self_position() const { return this->position_; }
        Quat4f get_self_rotation() const { return this->rotation_; }
        Vector3f get_self_euler_rotation() const { return this->rotation_.GetEuler(); }
        Vector3f get_self_scale() const { return this->scale_; }

        void set_self_position(Vector3f value) { this->position_ = value; }
        void set_self_rotation(Quat4f value) { this->rotation_ = value; }
        void set_self_euler_rotation(Vector3f value) { this->rotation_ = Quat4f(value); }
        void set_self_scale(Vector3f value) { this->scale_ = value; }
    public:
        Node(const Node& r) = delete;
        Node(Node&& r) = delete;
        Node& operator=(const Node& r) = delete;
    public:
        Node();

        virtual void Serialize(ser::Stream& stream, bool is_ser) override;

        //ITickable 
        virtual void OnTick(Ticker ticker) override;
    public:
        virtual void OnConstruct() override;
        virtual void OnDestroy() override;
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
