#include "Node.h"
#include <Apatite/Node.h>
#include <Apatite/Components/Component.h>
#include <Apatite/TransformUtil.h>
#include <stack>
#include <Apatite/Logger.h>

using namespace std;

namespace apatite
{

    bool Node::get_is_active() const
    {
        auto node = self();
        while (node != nullptr)
        {
            if (!node->get_is_active())
            {
                return false;
            }
            node = this->get_parent().lock();
        }
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
    //    assert(this->components_ && this->components_->size() != 0);
    //    auto& transform = this->components_->at(0);
    //    assert(cltypeof<TransformComponent>()->IsInstanceOfType(transform.get()));
    //    return sptr_cast<TransformComponent>(transform);
    //}

    Node_wp Node::get_parent() const
    {
        return this->parent_;
    }
    void Node::set_parent(sptr<Node> parent, bool keep_world_transform)
    {
        if (!this->has_parent_) // top node
        {
            if (parent != nullptr)
            {
                parent->childs_->push_back(self());
                this->has_parent_ = true;
                if (keep_world_transform)
                {
                    // todo transform matrix

                }
            }
        }
        else // has parent
        {
            // remove 
            auto my_parent = this->parent_.lock();
            assert(my_parent);
            auto it = std::find(my_parent->childs_->begin(), my_parent->childs_->end(), self());
            assert(it != my_parent->childs_->end());
            my_parent->childs_->erase(it);

            if (parent == nullptr) //set empty parent
            {
                this->has_parent_ = false;
            }
            else
            {
                parent->childs_->push_back(self());
            }
        }

        this->parent_ = parent;
    }

    int32_t Node::get_child_count() const
    {
        return this->childs_->size();
    }

    Vector3f Node::get_world_position() const
    {
        Vector3f pos = this->get_self_position();
        Node_sp node;
        if (this->has_parent())
        {
            node = this->get_parent().lock();
            auto v4 = node->GetLocalTransformMatrix() * (Vector4f)pos;
            pos = { v4.x, v4.y, v4.z };
        }
        return pos;
    }

    Quat4f Node::get_world_rotation() const
    {
        Quat4f rot = this->get_self_rotation();
        auto node = this->get_parent().lock();
        while (node)
        {
            rot *= node->get_self_rotation();
            node = node->get_parent().lock();
        }
        return rot;
    }

    Vector3f Node::get_world_euler_rotation() const
    {
        return this->get_world_rotation().GetEuler();
    }

    Vector3f Node::get_world_scale() const
    {
        Vector3f scale = this->get_self_scale();
        auto node = this->get_parent().lock();
        while (node)
        {
            scale = Vector3f::Mul(scale, node->get_self_scale());
            node = node->get_parent().lock();
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
        this->childs_ = mksptr(new List<Node_sp>);
        this->components_ = mksptr(new List<Component_sp>);
    }

    void Node::Serialize(ser::Stream& stream, bool is_ser)
    {
    }

    void Node::OnConstruct()
    {

    }

    void Node::OnDestroy()
    {

    }



    Component_sp Node::AddComponent(Type* type)
    {
        Object_sp obj = type->CreateSharedInstance({});
        Component_sp component = sptr_cast<Component>(obj);
        //init
        component->node_ = self_weak();
        this->components_->push_back(component);
        component->Construct();
        return component;
    }


    Component_sp Node::GetComponent(Type* type) const
    {
        for (auto& item : *this->components_)
        {
            if (type->IsInstanceOfType(item.get()))
            {
                return item;
            }
        }
        return nullptr;
    }

    void Node::GetAllComponents(List_sp<Component_sp>& list)
    {
        for (auto& item : *this->components_)
        {
            list->push_back(item);
        }
    }
    array_list<Component_sp> Node::GetAllComponentArray() const
    {
        return *this->components_;
    }
    void Node::GetChildren(List_sp<Node_sp>& list)
    {
        for (auto& item : *this->childs_)
        {
            list->push_back(item);
        }
    }
    void Node::OnInstantiateAsset(sptr<AssetObject>& obj)
    {
        auto that = sptr_cast<Node>(obj);

        for (auto& item : *this->childs_)
        {
            auto n = sptr_cast<Node>(item->InstantiateAsset());
            n->parent_ = that;
            that->childs_->push_back(n);
        }

        //todo: ser component panel data
        //for (auto& item : *this->components_)
        //{
        //    that->components_->push_back(sptr_cast<Component>(item->InstantiateAsset()));
        //}
    }
    void Node::Translate(Vector3f v)
    {
    }
    void Node::Rotate(Quat4f v)
    {
    }
    void Node::RotateEuler(Vector3f v)
    {
    }
    void Node::Scale(Vector3f v)
    {
    }
    Matrix4f Node::GetLocalTransformMatrix() const
    {
        return math::Translate(this->position_) * math::Rotate(this->rotation_) * math::Scale(this->scale_);
    }

    void Node::OnTick(Ticker ticker)
    {
        for (auto& comp : *this->components_)
        {
            if (comp->IsAlive())
            {
                comp->OnTick(ticker);
            }
        }
        for (auto& node : *this->childs_)
        {
            if (node->IsAlive())
            {
                node->OnTick(ticker);
            }
        }
    }

    Node_sp Node::GetChild(string_view name)
    {
        for (auto& item : *this->childs_)
        {
            if (item->name_ == name)
            {
                return item;
            }
        }
        return nullptr;
    }

    Node_sp Node::GetChildAt(int index)
    {
        return this->childs_->at(index);
    }

    Node_sp Node::StaticCreate(string_view name)
    {
        Node_sp node = mksptr(new Node);
        node->Construct();
        node->set_name(name);

        return node;
    }

}

