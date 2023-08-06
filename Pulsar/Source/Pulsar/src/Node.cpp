#include "Node.h"
#include <Pulsar/Components/Component.h>
#include <Pulsar/TransformUtil.h>
#include <stack>
#include <Pulsar/Logger.h>

using namespace std;

namespace pulsar
{

    bool Node::get_is_active() const
    {
        //auto node = self();
        //while (node != nullptr)
        //{
        //    if (!node->get_is_active())
        //    {
        //        return false;
        //    }
        //    node = this->GetParent();
        //}
        return true;
    }

    bool Node::get_is_active_self() const
    {
        return this->is_active_;
    }
    void Node::set_is_active_self(bool value)
    {
        this->is_active_ = value;
    }

    //TransformComponent_sp Node::get_transform()
    //{
    //    assert(this->m_components && this->m_components->size() != 0);
    //    auto& transform = this->m_components->at(0);
    //    assert(cltypeof<TransformComponent>()->IsInstanceOfType(transform.get()));
    //    return sptr_cast<TransformComponent>(transform);
    //}

    ObjectPtr<Node> Node::GetParent() const
    {
        return this->m_parent;
    }
    void Node::SetParent(ObjectPtr<Node> parent, bool keepWorldTransform)
    {
        if (!this->has_parent_) // top node
        {
            if (parent)
            {
                parent->m_children->push_back(self_ref());
                this->has_parent_ = true;
                if (keepWorldTransform)
                {
                    // todo transform matrix

                }
            }
        }
        else // has parent
        {
            // remove 
            auto my_parent = this->m_parent;
            assert(my_parent);
            auto it = std::find(my_parent->m_children->begin(), my_parent->m_children->end(), self_ref());
            assert(it != my_parent->m_children->end());
            my_parent->m_children->erase(it);

            if (parent) //set empty parent
            {
                this->has_parent_ = false;
            }
            else
            {
                parent->m_children->push_back(self_ref());
            }
        }

        this->m_parent = parent;
    }

    ObjectPtr<Node> Node::NewChildNode(string_view name)
    {
        auto newNode = StaticCreate(name);
        newNode->SetParent(self_ref());
        return newNode;
    }

    int32_t Node::get_child_count() const
    {
        return this->m_children->size();
    }

    Vector3f Node::get_world_position() const
    {
        return {};
        //Vector3f pos = this->get_self_position();
        //auto node = self();
        //if (node->has_parent())
        //{
        //    node = this->GetParent();
        //    auto mat = node->GetLocalTransformMatrix();
        //    //mat[3][0] = 0;
        //    //mat[3][1] = 0;
        //    //mat[3][2] = 0;
        //    //mat[3][3] = 1;
        //    auto v4 = mat * Vector4f(pos.x, pos.y, pos.z, 1);
        //    pos = { v4.x, v4.y, v4.z };
        //}
        //return pos;
    }
    //static Vector3f chgsign(const Vector3f& x, const Vector3f& y)
    //{
    //    return chgsign(float4(as_native(x)), float4(as_native(y))).xyz;
    //}
    //static Vector4f scaleMulQuat(const Vector3f& scale, const Vector4f& q)
    //{
    //    Vector3f s = chgsign(Vector3f(1,1,1), scale);
    //    return chgsign(q, Vector4f(s.yxx * s.zzy, 0));
    //}
    static Quat4f _ScaleMulQuat(const Vector3f& scale, const Quat4f& q)
    {
        auto s = jmath::Chgsign(Vector3f::Identity(), scale);
        return jmath::Chgsign(q, Quat4f(s.y * s.z, s.x * s.z, s.x * s.y, 0));
    }
    static Quat4f _QuatMul(const Quat4f& q1, const Quat4f& q2)
    {
        auto q = Quat4f(q1.y * q2.x, q1.w * q2.w, q1.z * q2.y, q1.x * q2.z) - Quat4f(q1.w * q2.z, q1.x * q2.x, q1.y * q2.z, q1.z * q2.x)
            - Quat4f(q1.z * q2.w, q1.z * q2.z, q1.w * q2.x, q1.w * q2.y) - Quat4f(q1.x * q2.y, q1.y * q2.y, q1.x * q2.w, q1.y * q2.w);
        return jmath::Chgsign(Quat4f(q.z, q.w, q.x, q.y), Quat4f(-1.f, -1.f, -1.f, 1.f));
    }
    Quat4f Node::get_world_rotation() const
    {
        Quat4f rot = this->get_self_rotation();
        auto node = this->GetParent();
        while (node)
        {
            rot = _ScaleMulQuat(node->scale_, rot);
            rot = _QuatMul(node->rotation_, rot);

            node = node->GetParent();
        }
        return rot;
    }

    void Node::set_world_rotation(const Quat4f& q)
    {

    }

    Vector3f Node::get_self_euler_rotation() const
    {
        return this->rotation_.GetEuler();
    }

    void Node::set_self_euler_rotation(const Vector3f& value)
    {
        this->rotation_ = Quat4f::FromEuler(value);
    }

    Vector3f Node::get_world_euler_rotation() const
    {
        return this->get_world_rotation().GetEuler();
    }

    Vector3f Node::get_world_scale() const
    {
        Vector3f scale = this->get_self_scale();
        auto node = this->GetParent();
        while (node)
        {
            scale = Vector3f::Mul(scale, node->get_self_scale());
            node = node->GetParent();
        }
        return scale;
    }

    Matrix4f Node::GetModelMatrix() const
    {
        assert(false);
        Matrix4f ret = Matrix4f::StaticScalar();
        transutil::Translate(&ret, this->get_world_position());
        transutil::Rotate(&ret, this->get_self_rotation());
        transutil::Scale(&ret, this->get_world_scale());

        return ret;
    }

    Vector3f Node::GetForward() const
    {
        auto rot = this->get_world_euler_rotation();
        rot.y -= 90.f;
        float x = cos(math::Radians(rot.y)) * cos(math::Radians(rot.x));
        float y = sin(math::Radians(rot.x));
        float z = sin(math::Radians(rot.y)) * cos(math::Radians(rot.x));
        return Vector3f::Normalize({ x,y,z });
    }


    Node::Node()
    {
        this->m_children = mksptr(new List<ObjectPtr<Node>>);
        this->m_components = mksptr(new List<ObjectPtr<Component>>);
        this->rotation_ = Quat4f::FromEuler({ 0,0,0 });
    }


    void Node::OnConstruct()
    {

    }

    void Node::OnDestroy()
    {
        base::OnDestroy();
        for (auto& comp : *m_components)
        {
            DestroyObject(comp);
        }
    }



    ObjectPtr<Component> Node::AddComponent(Type* type)
    {
        Object_sp obj = type->CreateSharedInstance({});
        Component_sp component = sptr_cast<Component>(obj);
        component->Construct();

        //init
        component->m_attachedNode = self_weak();
        component->m_ownerNode = self_weak();
        this->m_components->push_back(component);

        return component;
    }


    ObjectPtr<Component> Node::GetComponent(Type* type) const
    {
        for (auto& item : *this->m_components)
        {
            if (type->IsInstanceOfType(item.GetPtr()))
            {
                return item;
            }
        }
        return nullptr;
    }

    void Node::GetAllComponents(List_sp<ObjectPtr<Component>>& list)
    {
        for (auto& item : *this->m_components)
        {
            list->push_back(item);
        }
    }
    array_list<ObjectPtr<Component>> Node::GetAllComponentArray() const
    {
        return *this->m_components;
    }
    void Node::GetChildren(List_sp<ObjectPtr<Node>>& list)
    {
        for (auto& item : *this->m_children)
        {
            list->push_back(item);
        }
    }
    array_list<ObjectPtr<Node>> Node::GetChildrenArray()
    {
        array_list<ObjectPtr<Node>> ret;
        for (auto& item : *this->m_children)
        {
            ret.push_back(item);
        }
        return ret;
    }
    void Node::OnInstantiateAsset(ObjectPtr<AssetObject>& obj)
    {
        //auto that = sptr_cast<Node>(obj);

        //for (auto& item : *this->m_children)
        //{
        //    auto n = sptr_cast<Node>(item->InstantiateAsset());
        //    n->m_parent = that;
        //    that->m_children->push_back(n);
        //}

        //todo: ser component panel data
        //for (auto& item : *this->m_components)
        //{
        //    that->m_components->push_back(sptr_cast<Component>(item->InstantiateAsset()));
        //}
    }
    void Node::SendMessage(MessageId id)
    {
        for (auto& comp : *this->m_components)
        {
            comp->OnReceiveMessage(id);
        }
    }
    void Node::RotateEulerLocal(Vector3f v)
    {
        this->rotation_ *= Quat4f::FromEuler(v);
    }
    void Node::RotateEulerWorld(Vector3f v)
    {
        auto w_rot = this->get_world_rotation();
        this->set_world_rotation(w_rot * (jmath::Inverse(w_rot) * Quat4f::FromEuler(v) * w_rot));
    }

    Matrix4f Node::GetLocalTransformMatrix() const
    {
        return math::Translate(this->position_) * math::Rotate(this->rotation_) * math::Scale(this->scale_);
    }

    void Node::OnTick(Ticker ticker)
    {
        for (auto& comp : *this->m_components)
        {
            if (comp->IsAlive())
            {
                comp->OnTick(ticker);
            }
        }
        for (auto& node : *this->m_children)
        {
            if (node->IsAlive())
            {
                node->OnTick(ticker);
            }
        }
    }

    ObjectPtr<Node> Node::GetChild(string_view name)
    {
        for (auto& item : *this->m_children)
        {
            if (item->name_ == name)
            {
                return item;
            }
        }
        return nullptr;
    }

    ObjectPtr<Node> Node::GetChildAt(int index)
    {
        return this->m_children->at(index);
    }

    ObjectPtr<Node> Node::StaticCreate(string_view name)
    {
        Node_sp node = mksptr(new Node);
        node->Construct();
        node->SetName(name);

        return node;
    }

}

